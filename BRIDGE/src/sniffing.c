/*
 * Sniffing module for BRIDGE - Virtual Null Modem Bridge
 * Handles data capture, filtering, and streaming to multiple outputs
 */

#include "sniffing.h"
#include "utils.h"
#include <sys/stat.h>
#include <fcntl.h>

gboolean init_sniffing(BridgeApp *app) {
    // Initialize sniffing settings with defaults
    app->sniffing_enabled = FALSE;
    app->sniff_output_methods = SNIFF_OUTPUT_NONE;
    app->sniff_direction = SNIFF_DIRECTION_BOTH;
    app->sniff_format = SNIFF_FORMAT_RAW;
    
    strncpy(app->sniff_pipe_path, DEFAULT_SNIFF_PIPE, MAX_PATH_LENGTH - 1);
    app->sniff_tcp_port = DEFAULT_SNIFF_TCP_PORT;
    app->sniff_udp_port = DEFAULT_SNIFF_UDP_PORT;
    strncpy(app->sniff_udp_addr, DEFAULT_SNIFF_UDP_ADDR, 63);
    
    // Initialize runtime state
    app->sniff_thread_running = FALSE;
    app->sniff_pipe_fd = -1;
    app->sniff_tcp_server_fd = -1;
    app->sniff_udp_fd = -1;
    app->sniff_log_fp = NULL;
    
    // Initialize client connections
    for (int i = 0; i < MAX_SNIFF_CLIENTS; i++) {
        app->sniff_tcp_client_fds[i] = -1;
    }
    
    // Initialize statistics
    app->sniff_bytes_captured = 0;
    app->sniff_packets_sent = 0;
    app->sniff_start_time = 0;
    
    return TRUE;
}

void cleanup_sniffing(BridgeApp *app) {
    if (is_sniffing_active(app)) {
        stop_sniffing(app);
    }
    
    cleanup_sniff_pipe(app);
    cleanup_sniff_tcp_server(app);
    cleanup_sniff_udp_socket(app);
    cleanup_sniff_log_file(app);
}

gboolean start_sniffing(BridgeApp *app) {
    if (!app->sniffing_enabled || app->sniff_output_methods == SNIFF_OUTPUT_NONE) {
        log_message(app, "Sniffing not enabled or no output methods selected");
        return FALSE;
    }
    
    if (is_sniffing_active(app)) {
        log_message(app, "Sniffing is already active");
        return FALSE;
    }
    
    log_message(app, "Starting sniffing...");
    
    // Setup output methods
    gboolean setup_success = TRUE;
    
    if (app->sniff_output_methods & SNIFF_OUTPUT_PIPE) {
        setup_success &= setup_sniff_pipe(app);
    }
    
    if (app->sniff_output_methods & SNIFF_OUTPUT_TCP) {
        setup_success &= setup_sniff_tcp_server(app);
    }
    
    if (app->sniff_output_methods & SNIFF_OUTPUT_UDP) {
        setup_success &= setup_sniff_udp_socket(app);
    }
    
    if (app->sniff_output_methods & SNIFF_OUTPUT_FILE) {
        setup_success &= setup_sniff_log_file(app);
    }
    
    if (!setup_success) {
        log_message(app, "Failed to setup sniffing outputs");
        cleanup_sniffing(app);
        return FALSE;
    }
    
    // Start sniffing thread
    app->sniff_thread_running = TRUE;
    app->sniff_start_time = time(NULL);
    
    if (pthread_create(&app->sniff_thread, NULL, sniffing_thread_func, app) != 0) {
        log_message(app, "Failed to create sniffing thread");
        app->sniff_thread_running = FALSE;
        cleanup_sniffing(app);
        return FALSE;
    }
    
    log_message(app, "✓ Sniffing started successfully");
    return TRUE;
}

void stop_sniffing(BridgeApp *app) {
    if (!is_sniffing_active(app)) {
        return;
    }
    
    log_message(app, "Stopping sniffing...");
    
    // Stop sniffing thread
    app->sniff_thread_running = FALSE;
    pthread_join(app->sniff_thread, NULL);
    
    // Cleanup outputs
    cleanup_sniffing(app);
    
    log_message(app, "Sniffing stopped");
}

gboolean is_sniffing_active(BridgeApp *app) {
    return app->sniff_thread_running;
}

gboolean setup_sniff_pipe(BridgeApp *app) {
    // Remove existing pipe if it exists
    unlink(app->sniff_pipe_path);
    
    // Create named pipe
    if (mkfifo(app->sniff_pipe_path, 0666) != 0) {
        log_message(app, "Failed to create sniff pipe %s: %s", 
                   app->sniff_pipe_path, strerror(errno));
        return FALSE;
    }
    
    // Open pipe for writing (non-blocking)
    app->sniff_pipe_fd = open(app->sniff_pipe_path, O_WRONLY | O_NONBLOCK);
    if (app->sniff_pipe_fd < 0) {
        log_message(app, "Failed to open sniff pipe %s: %s", 
                   app->sniff_pipe_path, strerror(errno));
        unlink(app->sniff_pipe_path);
        return FALSE;
    }
    
    log_message(app, "✓ Sniff pipe created: %s", app->sniff_pipe_path);
    return TRUE;
}

gboolean setup_sniff_tcp_server(BridgeApp *app) {
    struct sockaddr_in server_addr;
    
    // Create TCP socket
    app->sniff_tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (app->sniff_tcp_server_fd < 0) {
        log_message(app, "Failed to create TCP socket: %s", strerror(errno));
        return FALSE;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(app->sniff_tcp_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(app->sniff_tcp_port);
    
    if (bind(app->sniff_tcp_server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        log_message(app, "Failed to bind TCP socket to port %d: %s", 
                   app->sniff_tcp_port, strerror(errno));
        close(app->sniff_tcp_server_fd);
        app->sniff_tcp_server_fd = -1;
        return FALSE;
    }
    
    // Listen for connections
    if (listen(app->sniff_tcp_server_fd, MAX_SNIFF_CLIENTS) < 0) {
        log_message(app, "Failed to listen on TCP socket: %s", strerror(errno));
        close(app->sniff_tcp_server_fd);
        app->sniff_tcp_server_fd = -1;
        return FALSE;
    }
    
    log_message(app, "✓ TCP server listening on port %d", app->sniff_tcp_port);
    return TRUE;
}

gboolean setup_sniff_udp_socket(BridgeApp *app) {
    struct sockaddr_in udp_addr;
    
    // Create UDP socket
    app->sniff_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (app->sniff_udp_fd < 0) {
        log_message(app, "Failed to create UDP socket: %s", strerror(errno));
        return FALSE;
    }
    
    // Set up broadcast address
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(app->sniff_udp_port);
    inet_pton(AF_INET, app->sniff_udp_addr, &udp_addr.sin_addr);
    
    log_message(app, "✓ UDP socket ready for %s:%d", app->sniff_udp_addr, app->sniff_udp_port);
    return TRUE;
}

gboolean setup_sniff_log_file(BridgeApp *app) {
    if (strlen(app->sniff_log_file) == 0) {
        // Generate default log filename with timestamp
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        snprintf(app->sniff_log_file, MAX_PATH_LENGTH, 
                "bridge_sniff_%04d%02d%02d_%02d%02d%02d.log",
                tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    }
    
    app->sniff_log_fp = fopen(app->sniff_log_file, "w");
    if (!app->sniff_log_fp) {
        log_message(app, "Failed to open log file %s: %s", 
                   app->sniff_log_file, strerror(errno));
        return FALSE;
    }
    
    log_message(app, "✓ Sniff log file opened: %s", app->sniff_log_file);
    return TRUE;
}

void cleanup_sniff_pipe(BridgeApp *app) {
    if (app->sniff_pipe_fd >= 0) {
        close(app->sniff_pipe_fd);
        app->sniff_pipe_fd = -1;
        unlink(app->sniff_pipe_path);
    }
}

void cleanup_sniff_tcp_server(BridgeApp *app) {
    cleanup_tcp_clients(app);
    
    if (app->sniff_tcp_server_fd >= 0) {
        close(app->sniff_tcp_server_fd);
        app->sniff_tcp_server_fd = -1;
    }
}

void cleanup_sniff_udp_socket(BridgeApp *app) {
    if (app->sniff_udp_fd >= 0) {
        close(app->sniff_udp_fd);
        app->sniff_udp_fd = -1;
    }
}

void cleanup_sniff_log_file(BridgeApp *app) {
    if (app->sniff_log_fp) {
        fclose(app->sniff_log_fp);
        app->sniff_log_fp = NULL;
    }
}

void* sniffing_thread_func(void *arg) {
    BridgeApp *app = (BridgeApp *)arg;

    log_message(app, "Sniffing thread started");

    while (app->sniff_thread_running) {
        // Accept new TCP clients if TCP output is enabled
        if (app->sniff_output_methods & SNIFF_OUTPUT_TCP) {
            accept_tcp_clients(app);
        }

        // Sleep briefly to avoid busy waiting
        usleep(100000); // 100ms
    }

    log_message(app, "Sniffing thread stopped");
    return NULL;
}

void process_sniff_data(BridgeApp *app, const char *data, size_t len, char direction) {
    if (!is_sniffing_active(app) || !should_capture_direction(app, direction)) {
        return;
    }

    // Create sniff packet
    SniffPacket packet;
    packet.timestamp = time(NULL);
    packet.direction = direction;
    packet.data_len = (len > MAX_SNIFF_BUFFER_SIZE) ? MAX_SNIFF_BUFFER_SIZE : len;
    memcpy(packet.data, data, packet.data_len);

    // Stream to all configured outputs
    stream_to_outputs(app, &packet);

    // Update statistics
    update_sniff_statistics(app, packet.data_len);
}

void stream_to_outputs(BridgeApp *app, const SniffPacket *packet) {
    char *formatted_data = format_sniff_data(packet, app->sniff_format);
    if (!formatted_data) {
        return;
    }

    // Send to each enabled output method
    if (app->sniff_output_methods & SNIFF_OUTPUT_PIPE) {
        write_to_pipe(app, formatted_data);
    }

    if (app->sniff_output_methods & SNIFF_OUTPUT_TCP) {
        write_to_tcp_clients(app, formatted_data);
    }

    if (app->sniff_output_methods & SNIFF_OUTPUT_UDP) {
        write_to_udp(app, formatted_data);
    }

    if (app->sniff_output_methods & SNIFF_OUTPUT_FILE) {
        write_to_log_file(app, formatted_data);
    }

    free(formatted_data);
}

char* format_sniff_data(const SniffPacket *packet, SniffFormat format) {
    char *result = NULL;
    size_t result_size = 0;

    // Calculate required buffer size based on format
    switch (format) {
        case SNIFF_FORMAT_RAW:
            result_size = packet->data_len + 1;
            result = malloc(result_size);
            if (result) {
                memcpy(result, packet->data, packet->data_len);
                result[packet->data_len] = '\0';
            }
            break;

        case SNIFF_FORMAT_HEX:
            // Format: "TIMESTAMP DIR: HEX_DATA\n"
            result_size = 64 + (packet->data_len * 3) + 1;
            result = malloc(result_size);
            if (result) {
                char timestamp_str[32];
                struct tm *tm_info = localtime(&packet->timestamp);
                strftime(timestamp_str, sizeof(timestamp_str), "%H:%M:%S", tm_info);

                int offset = snprintf(result, result_size, "%s %c: ", timestamp_str, packet->direction);

                for (size_t i = 0; i < packet->data_len; i++) {
                    offset += snprintf(result + offset, result_size - offset, "%02X ",
                                     (unsigned char)packet->data[i]);
                }
                snprintf(result + offset, result_size - offset, "\n");
            }
            break;

        case SNIFF_FORMAT_TEXT:
            // Format: "TIMESTAMP DIR: TEXT_DATA\n"
            result_size = 64 + packet->data_len + 1;
            result = malloc(result_size);
            if (result) {
                char timestamp_str[32];
                struct tm *tm_info = localtime(&packet->timestamp);
                strftime(timestamp_str, sizeof(timestamp_str), "%H:%M:%S", tm_info);

                snprintf(result, result_size, "%s %c: %.*s\n",
                        timestamp_str, packet->direction,
                        (int)packet->data_len, packet->data);
            }
            break;
    }

    return result;
}

void write_to_pipe(BridgeApp *app, const char *formatted_data) {
    if (app->sniff_pipe_fd >= 0) {
        ssize_t written = write(app->sniff_pipe_fd, formatted_data, strlen(formatted_data));
        if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            // Pipe error (reader disconnected, etc.)
            log_message(app, "Pipe write error: %s", strerror(errno));
        }
    }
}

void write_to_tcp_clients(BridgeApp *app, const char *formatted_data) {
    size_t data_len = strlen(formatted_data);

    for (int i = 0; i < MAX_SNIFF_CLIENTS; i++) {
        if (app->sniff_tcp_client_fds[i] >= 0) {
            ssize_t written = write(app->sniff_tcp_client_fds[i], formatted_data, data_len);
            if (written < 0) {
                // Client disconnected or error
                close(app->sniff_tcp_client_fds[i]);
                app->sniff_tcp_client_fds[i] = -1;
            }
        }
    }
}

void write_to_udp(BridgeApp *app, const char *formatted_data) {
    if (app->sniff_udp_fd >= 0) {
        struct sockaddr_in udp_addr;
        memset(&udp_addr, 0, sizeof(udp_addr));
        udp_addr.sin_family = AF_INET;
        udp_addr.sin_port = htons(app->sniff_udp_port);
        inet_pton(AF_INET, app->sniff_udp_addr, &udp_addr.sin_addr);

        sendto(app->sniff_udp_fd, formatted_data, strlen(formatted_data), 0,
               (struct sockaddr*)&udp_addr, sizeof(udp_addr));
    }
}

void write_to_log_file(BridgeApp *app, const char *formatted_data) {
    if (app->sniff_log_fp) {
        fprintf(app->sniff_log_fp, "%s", formatted_data);
        fflush(app->sniff_log_fp);
    }
}

void accept_tcp_clients(BridgeApp *app) {
    if (app->sniff_tcp_server_fd < 0) {
        return;
    }

    // Set socket to non-blocking for accept
    int flags = fcntl(app->sniff_tcp_server_fd, F_GETFL, 0);
    fcntl(app->sniff_tcp_server_fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(app->sniff_tcp_server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd >= 0) {
        // Find empty slot for new client
        for (int i = 0; i < MAX_SNIFF_CLIENTS; i++) {
            if (app->sniff_tcp_client_fds[i] < 0) {
                app->sniff_tcp_client_fds[i] = client_fd;
                log_message(app, "TCP client connected: %s", inet_ntoa(client_addr.sin_addr));
                break;
            }
        }

        // If no slots available, close connection
        if (client_fd >= 0) {
            gboolean slot_found = FALSE;
            for (int i = 0; i < MAX_SNIFF_CLIENTS; i++) {
                if (app->sniff_tcp_client_fds[i] == client_fd) {
                    slot_found = TRUE;
                    break;
                }
            }
            if (!slot_found) {
                close(client_fd);
                log_message(app, "TCP client rejected: maximum clients reached");
            }
        }
    }

    // Restore blocking mode
    fcntl(app->sniff_tcp_server_fd, F_SETFL, flags);
}

void cleanup_tcp_clients(BridgeApp *app) {
    for (int i = 0; i < MAX_SNIFF_CLIENTS; i++) {
        if (app->sniff_tcp_client_fds[i] >= 0) {
            close(app->sniff_tcp_client_fds[i]);
            app->sniff_tcp_client_fds[i] = -1;
        }
    }
}

gboolean should_capture_direction(BridgeApp *app, char direction) {
    switch (app->sniff_direction) {
        case SNIFF_DIRECTION_BOTH:
            return TRUE;
        case SNIFF_DIRECTION_RX_ONLY:
            return (direction == 'R');
        case SNIFF_DIRECTION_TX_ONLY:
            return (direction == 'T');
        default:
            return FALSE;
    }
}

void update_sniff_statistics(BridgeApp *app, size_t bytes_processed) {
    app->sniff_bytes_captured += bytes_processed;
    app->sniff_packets_sent++;
}
