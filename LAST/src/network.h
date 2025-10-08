#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

/*
 * Network communication module for LAST - Linux Advanced Serial Transceiver
 * Handles TCP/UDP client and server connections for NMEA 0183 over Ethernet
 */

// Network connection management functions
gboolean connect_tcp_client(SerialTerminal *terminal, const char *host, int port);
gboolean connect_tcp_server(SerialTerminal *terminal, int port);
gboolean connect_udp_client(SerialTerminal *terminal, const char *host, int port);
gboolean connect_udp_server(SerialTerminal *terminal, int port);
void disconnect_network(SerialTerminal *terminal);

// Network I/O functions
ssize_t network_send_data(SerialTerminal *terminal, const void *data, size_t len);
void *network_read_thread_func(void *arg);

// Network utility functions
const char* connection_type_to_string(ConnectionType type);
ConnectionType string_to_connection_type(const char *str);
gboolean is_valid_hostname(const char *hostname);
gboolean is_valid_port(const char *port_str);

// Network status functions
void show_network_status(SerialTerminal *terminal, const char *message);
char* get_network_connection_info(SerialTerminal *terminal);

#endif // NETWORK_H
