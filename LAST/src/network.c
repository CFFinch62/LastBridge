/*
 * Network communication module for LAST - Linux Advanced Serial Transceiver
 * Handles TCP/UDP client and server connections for NMEA 0183 over Ethernet
 */

#include "network.h"
#include "utils.h"
#include "serial.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

const char* connection_type_to_string(ConnectionType type) {
    switch (type) {
        case CONNECTION_TYPE_SERIAL: return "Serial";
        case CONNECTION_TYPE_TCP_CLIENT: return "TCP Client";
        case CONNECTION_TYPE_TCP_SERVER: return "TCP Server";
        case CONNECTION_TYPE_UDP_CLIENT: return "UDP Client";
        case CONNECTION_TYPE_UDP_SERVER: return "UDP Server";
        default: return "Unknown";
    }
}

ConnectionType string_to_connection_type(const char *str) {
    if (!str) return CONNECTION_TYPE_SERIAL;
    
    if (strcmp(str, "TCP Client") == 0) return CONNECTION_TYPE_TCP_CLIENT;
    if (strcmp(str, "TCP Server") == 0) return CONNECTION_TYPE_TCP_SERVER;
    if (strcmp(str, "UDP Client") == 0) return CONNECTION_TYPE_UDP_CLIENT;
    if (strcmp(str, "UDP Server") == 0) return CONNECTION_TYPE_UDP_SERVER;
    
    return CONNECTION_TYPE_SERIAL;
}

gboolean is_valid_hostname(const char *hostname) {
    if (!hostname || strlen(hostname) == 0) return FALSE;
    if (strlen(hostname) >= MAX_HOSTNAME_LENGTH) return FALSE;
    
    // Basic validation - could be enhanced
    return TRUE;
}

gboolean is_valid_port(const char *port_str) {
    if (!port_str || strlen(port_str) == 0) return FALSE;
    
    char *endptr;
    long port = strtol(port_str, &endptr, 10);
    
    if (*endptr != '\0') return FALSE;  // Non-numeric characters
    if (port < 1 || port > 65535) return FALSE;  // Invalid port range
    
    return TRUE;
}

gboolean connect_tcp_client(SerialTerminal *terminal, const char *host, int port) {
    struct sockaddr_in server_addr;
    struct hostent *server;
    
    // Create socket
    terminal->connection_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (terminal->connection_fd < 0) {
        show_network_status(terminal, "Failed to create TCP socket");
        return FALSE;
    }
    
    // Resolve hostname
    server = gethostbyname(host);
    if (server == NULL) {
        close(terminal->connection_fd);
        terminal->connection_fd = -1;
        show_network_status(terminal, "Failed to resolve hostname");
        return FALSE;
    }
    
    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(port);
    
    // Connect to server
    if (connect(terminal->connection_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(terminal->connection_fd);
        terminal->connection_fd = -1;
        show_network_status(terminal, "Failed to connect to TCP server");
        return FALSE;
    }
    
    return TRUE;
}

gboolean connect_tcp_server(SerialTerminal *terminal, int port) {
    int opt = 1;
    
    // Create socket
    terminal->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (terminal->server_fd < 0) {
        show_network_status(terminal, "Failed to create TCP server socket");
        return FALSE;
    }
    
    // Set socket options
    if (setsockopt(terminal->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(terminal->server_fd);
        terminal->server_fd = -1;
        show_network_status(terminal, "Failed to set socket options");
        return FALSE;
    }
    
    // Setup server address
    memset(&terminal->server_addr, 0, sizeof(terminal->server_addr));
    terminal->server_addr.sin_family = AF_INET;
    terminal->server_addr.sin_addr.s_addr = INADDR_ANY;
    terminal->server_addr.sin_port = htons(port);
    
    // Bind socket
    if (bind(terminal->server_fd, (struct sockaddr*)&terminal->server_addr, sizeof(terminal->server_addr)) < 0) {
        close(terminal->server_fd);
        terminal->server_fd = -1;
        show_network_status(terminal, "Failed to bind TCP server socket");
        return FALSE;
    }
    
    // Listen for connections
    if (listen(terminal->server_fd, 1) < 0) {
        close(terminal->server_fd);
        terminal->server_fd = -1;
        show_network_status(terminal, "Failed to listen on TCP server socket");
        return FALSE;
    }
    
    // Accept connection (blocking)
    terminal->client_addr_len = sizeof(terminal->client_addr);
    terminal->connection_fd = accept(terminal->server_fd, (struct sockaddr*)&terminal->client_addr, &terminal->client_addr_len);
    if (terminal->connection_fd < 0) {
        close(terminal->server_fd);
        terminal->server_fd = -1;
        show_network_status(terminal, "Failed to accept TCP connection");
        return FALSE;
    }
    
    return TRUE;
}

gboolean connect_udp_client(SerialTerminal *terminal, const char *host, int port) {
    struct hostent *server;
    
    // Create socket
    terminal->connection_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (terminal->connection_fd < 0) {
        show_network_status(terminal, "Failed to create UDP socket");
        return FALSE;
    }
    
    // Resolve hostname
    server = gethostbyname(host);
    if (server == NULL) {
        close(terminal->connection_fd);
        terminal->connection_fd = -1;
        show_network_status(terminal, "Failed to resolve hostname");
        return FALSE;
    }
    
    // Setup server address for UDP client
    memset(&terminal->server_addr, 0, sizeof(terminal->server_addr));
    terminal->server_addr.sin_family = AF_INET;
    memcpy(&terminal->server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    terminal->server_addr.sin_port = htons(port);
    
    return TRUE;
}

gboolean connect_udp_server(SerialTerminal *terminal, int port) {
    // Create socket
    terminal->connection_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (terminal->connection_fd < 0) {
        show_network_status(terminal, "Failed to create UDP server socket");
        return FALSE;
    }
    
    // Setup server address
    memset(&terminal->server_addr, 0, sizeof(terminal->server_addr));
    terminal->server_addr.sin_family = AF_INET;
    terminal->server_addr.sin_addr.s_addr = INADDR_ANY;
    terminal->server_addr.sin_port = htons(port);
    
    // Bind socket
    if (bind(terminal->connection_fd, (struct sockaddr*)&terminal->server_addr, sizeof(terminal->server_addr)) < 0) {
        close(terminal->connection_fd);
        terminal->connection_fd = -1;
        show_network_status(terminal, "Failed to bind UDP server socket");
        return FALSE;
    }
    
    return TRUE;
}

void disconnect_network(SerialTerminal *terminal) {
    if (terminal->connection_fd >= 0) {
        close(terminal->connection_fd);
        terminal->connection_fd = -1;
    }
    
    if (terminal->server_fd >= 0) {
        close(terminal->server_fd);
        terminal->server_fd = -1;
    }
}

ssize_t network_send_data(SerialTerminal *terminal, const void *data, size_t len) {
    if (terminal->connection_fd < 0) return -1;
    
    switch (terminal->connection_type) {
        case CONNECTION_TYPE_TCP_CLIENT:
        case CONNECTION_TYPE_TCP_SERVER:
            return send(terminal->connection_fd, data, len, 0);
            
        case CONNECTION_TYPE_UDP_CLIENT:
            return sendto(terminal->connection_fd, data, len, 0, 
                         (struct sockaddr*)&terminal->server_addr, sizeof(terminal->server_addr));
            
        case CONNECTION_TYPE_UDP_SERVER:
            // For UDP server, send to last known client
            if (terminal->client_addr_len > 0) {
                return sendto(terminal->connection_fd, data, len, 0,
                             (struct sockaddr*)&terminal->client_addr, terminal->client_addr_len);
            }
            return -1;
            
        default:
            return -1;
    }
}

void show_network_status(SerialTerminal *terminal, const char *message) {
    if (terminal && terminal->status_label) {
        gtk_label_set_text(GTK_LABEL(terminal->status_label), message);
    }
}

char* get_network_connection_info(SerialTerminal *terminal) {
    if (!terminal) return NULL;
    
    char *info = malloc(256);
    if (!info) return NULL;
    
    switch (terminal->connection_type) {
        case CONNECTION_TYPE_TCP_CLIENT:
            snprintf(info, 256, "TCP Client connected to %s:%s", 
                    terminal->network_host, terminal->network_port);
            break;
        case CONNECTION_TYPE_TCP_SERVER:
            snprintf(info, 256, "TCP Server listening on port %s", 
                    terminal->network_port);
            break;
        case CONNECTION_TYPE_UDP_CLIENT:
            snprintf(info, 256, "UDP Client connected to %s:%s", 
                    terminal->network_host, terminal->network_port);
            break;
        case CONNECTION_TYPE_UDP_SERVER:
            snprintf(info, 256, "UDP Server listening on port %s", 
                    terminal->network_port);
            break;
        default:
            snprintf(info, 256, "Unknown connection type");
            break;
    }
    
    return info;
}

void *network_read_thread_func(void *arg) {
    SerialTerminal *terminal = (SerialTerminal *)arg;
    char buffer[1024];

    while (terminal->thread_running) {
        fd_set readfds;
        struct timeval timeout;
        ssize_t bytes_read = 0;

        FD_ZERO(&readfds);
        FD_SET(terminal->connection_fd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100ms timeout

        int result = select(terminal->connection_fd + 1, &readfds, NULL, NULL, &timeout);
        if (result > 0 && FD_ISSET(terminal->connection_fd, &readfds)) {

            switch (terminal->connection_type) {
                case CONNECTION_TYPE_TCP_CLIENT:
                case CONNECTION_TYPE_TCP_SERVER:
                    bytes_read = recv(terminal->connection_fd, buffer, sizeof(buffer) - 1, 0);
                    break;

                case CONNECTION_TYPE_UDP_CLIENT:
                    bytes_read = recvfrom(terminal->connection_fd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
                    break;

                case CONNECTION_TYPE_UDP_SERVER:
                    terminal->client_addr_len = sizeof(terminal->client_addr);
                    bytes_read = recvfrom(terminal->connection_fd, buffer, sizeof(buffer) - 1, 0,
                                        (struct sockaddr*)&terminal->client_addr, &terminal->client_addr_len);
                    break;

                default:
                    bytes_read = -1;
                    break;
            }

            if (bytes_read > 0) {
                // Update statistics
                terminal->bytes_received += bytes_read;

                // Mark RX activity
                terminal->rx_active = TRUE;
                terminal->rx_last_activity = time(NULL);

                // Log to file if enabled
                if (terminal->log_file) {
                    char *timestamp = get_current_timestamp();
                    // For file logging, null-terminate for text output
                    buffer[bytes_read] = '\0';
                    fprintf(terminal->log_file, "[%s] RX: %s", timestamp, buffer);
                    fflush(terminal->log_file);
                    free(timestamp);
                }

                // Create dual display data structure (reuse from serial.c)
                DualDisplayData *dual_data = malloc(sizeof(DualDisplayData));
                dual_data->text_data = format_data_for_display(buffer, bytes_read, FALSE);
                dual_data->hex_data = format_data_for_display(buffer, bytes_read, TRUE);
                dual_data->show_hex = terminal->hex_display;
                g_idle_add(append_to_dual_display_idle, dual_data);

            } else if (bytes_read == 0) {
                // Connection closed by peer (TCP only)
                if (terminal->connection_type == CONNECTION_TYPE_TCP_CLIENT ||
                    terminal->connection_type == CONNECTION_TYPE_TCP_SERVER) {
                    show_network_status(terminal, "Connection closed by peer");
                    break;
                }
            } else if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                // Real error occurred
                show_network_status(terminal, "Network read error");
                break;
            }
        }
    }

    return NULL;
}
