/*
 * Sniffing module header for BRIDGE - Virtual Null Modem Bridge
 * Handles data capture, filtering, and streaming to multiple outputs
 */

#ifndef SNIFFING_H
#define SNIFFING_H

#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Sniffing data packet structure
typedef struct {
    time_t timestamp;
    char direction; // 'R' for RX, 'T' for TX
    size_t data_len;
    char data[MAX_SNIFF_BUFFER_SIZE];
} SniffPacket;

// Function declarations
gboolean init_sniffing(BridgeApp *app);
void cleanup_sniffing(BridgeApp *app);
gboolean start_sniffing(BridgeApp *app);
void stop_sniffing(BridgeApp *app);
gboolean is_sniffing_active(BridgeApp *app);

// Output method management
gboolean setup_sniff_pipe(BridgeApp *app);
gboolean setup_sniff_tcp_server(BridgeApp *app);
gboolean setup_sniff_udp_socket(BridgeApp *app);
gboolean setup_sniff_log_file(BridgeApp *app);

void cleanup_sniff_pipe(BridgeApp *app);
void cleanup_sniff_tcp_server(BridgeApp *app);
void cleanup_sniff_udp_socket(BridgeApp *app);
void cleanup_sniff_log_file(BridgeApp *app);

// Data processing and streaming
void* sniffing_thread_func(void *arg);
void process_sniff_data(BridgeApp *app, const char *data, size_t len, char direction);
void stream_to_outputs(BridgeApp *app, const SniffPacket *packet);

// Output formatting
char* format_sniff_data(const SniffPacket *packet, SniffFormat format);
void write_to_pipe(BridgeApp *app, const char *formatted_data);
void write_to_tcp_clients(BridgeApp *app, const char *formatted_data);
void write_to_udp(BridgeApp *app, const char *formatted_data);
void write_to_log_file(BridgeApp *app, const char *formatted_data);

// TCP client management
void accept_tcp_clients(BridgeApp *app);
void cleanup_tcp_clients(BridgeApp *app);

// Utility functions
gboolean should_capture_direction(BridgeApp *app, char direction);
void update_sniff_statistics(BridgeApp *app, size_t bytes_processed);

#endif // SNIFFING_H
