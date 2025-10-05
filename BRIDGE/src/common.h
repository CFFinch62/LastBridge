/*
 * Common header file for BRIDGE - Virtual Null Modem Bridge
 * Shared includes, constants, and main application structure
 */

#ifndef COMMON_H
#define COMMON_H

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

// Feature test macros for POSIX functions
#define _GNU_SOURCE

// GTK includes
#include <gtk/gtk.h>

// Application constants
#define MAX_PATH_LENGTH 256
#define MAX_LOG_LENGTH 1024
#define DEFAULT_DEVICE1 "/tmp/ttyV0"
#define DEFAULT_DEVICE2 "/tmp/ttyV1"
#define SOCAT_CHECK_INTERVAL 1000  // milliseconds
#define DEVICE_CREATION_TIMEOUT 5000  // milliseconds

// Sniffing constants
#define MAX_SNIFF_BUFFER_SIZE 4096
#define DEFAULT_SNIFF_PIPE "/tmp/bridge_sniff_pipe"
#define DEFAULT_SNIFF_TCP_PORT 8888
#define DEFAULT_SNIFF_UDP_PORT 9999
#define DEFAULT_SNIFF_UDP_ADDR "239.1.1.1"
#define MAX_SNIFF_CLIENTS 10

// Application states
typedef enum {
    BRIDGE_STATE_STOPPED,
    BRIDGE_STATE_STARTING,
    BRIDGE_STATE_RUNNING,
    BRIDGE_STATE_STOPPING,
    BRIDGE_STATE_ERROR
} BridgeState;

// Sniffing output methods
typedef enum {
    SNIFF_OUTPUT_NONE = 0,
    SNIFF_OUTPUT_PIPE = 1,
    SNIFF_OUTPUT_TCP = 2,
    SNIFF_OUTPUT_UDP = 4,
    SNIFF_OUTPUT_FILE = 8
} SniffOutputMethod;

// Sniffing data direction
typedef enum {
    SNIFF_DIRECTION_BOTH,
    SNIFF_DIRECTION_RX_ONLY,
    SNIFF_DIRECTION_TX_ONLY
} SniffDirection;

// Sniffing data format
typedef enum {
    SNIFF_FORMAT_RAW,
    SNIFF_FORMAT_HEX,
    SNIFF_FORMAT_TEXT
} SniffFormat;

// Main application data structure
typedef struct {
    // Main window and layout
    GtkWidget *window;
    GtkWidget *main_vbox;
    GtkWidget *notebook;

    // Configuration widgets
    GtkWidget *device1_entry;
    GtkWidget *device2_entry;
    GtkWidget *start_button;
    GtkWidget *stop_button;
    GtkWidget *test_button;
    GtkWidget *clear_log_button;

    // Status widgets
    GtkWidget *status_label;
    GtkWidget *devices_label;
    GtkWidget *connection_time_label;

    // Log display
    GtkWidget *log_text;
    GtkTextBuffer *log_buffer;

    // Settings widgets
    GtkWidget *auto_start_check;
    GtkWidget *verbose_logging_check;
    GtkWidget *device_permissions_combo;

    // Sniffing widgets
    GtkWidget *sniffing_enable_check;
    GtkWidget *sniff_pipe_check;
    GtkWidget *sniff_tcp_check;
    GtkWidget *sniff_udp_check;
    GtkWidget *sniff_file_check;
    GtkWidget *sniff_pipe_entry;
    GtkWidget *sniff_tcp_port_entry;
    GtkWidget *sniff_udp_port_entry;
    GtkWidget *sniff_udp_addr_entry;
    GtkWidget *sniff_file_entry;
    GtkWidget *sniff_direction_combo;
    GtkWidget *sniff_format_combo;
    GtkWidget *sniff_start_button;
    GtkWidget *sniff_stop_button;
    GtkWidget *sniff_stats_label;

    // Null modem state
    BridgeState state;
    pid_t socat_pid;
    char device1_path[MAX_PATH_LENGTH];
    char device2_path[MAX_PATH_LENGTH];
    time_t start_time;
    gboolean running;
    
    // Monitoring
    guint status_timer_id;
    pthread_t monitor_thread;
    gboolean monitor_running;

    // Statistics
    unsigned long test_count;
    unsigned long successful_tests;
    time_t last_test_time;

    // Settings
    gboolean auto_start;
    gboolean verbose_logging;
    char *device_permissions;  // "666", "644", etc.
    
    // Appearance settings (following LAST pattern)
    char *font_family;
    int font_size;
    char *bg_color;
    char *text_color;
    char *theme_preference;

    // Sniffing settings and state
    gboolean sniffing_enabled;
    SniffOutputMethod sniff_output_methods;
    SniffDirection sniff_direction;
    SniffFormat sniff_format;
    char sniff_pipe_path[MAX_PATH_LENGTH];
    int sniff_tcp_port;
    int sniff_udp_port;
    char sniff_udp_addr[64];
    char sniff_log_file[MAX_PATH_LENGTH];

    // Sniffing runtime state
    pthread_t sniff_thread;
    gboolean sniff_thread_running;
    int sniff_pipe_fd;
    int sniff_tcp_server_fd;
    int sniff_tcp_client_fds[MAX_SNIFF_CLIENTS];
    int sniff_udp_fd;
    FILE *sniff_log_fp;
    char capture_file_path[MAX_PATH_LENGTH];

    // Sniffing statistics
    unsigned long sniff_bytes_captured;
    unsigned long sniff_packets_sent;
    time_t sniff_start_time;

} BridgeApp;

// Global application instance (defined in main.c, declared here)
extern BridgeApp *g_bridge_app;

// Function declarations from other modules
// nullmodem.h
gboolean create_null_modem(BridgeApp *app);
void stop_null_modem(BridgeApp *app);
gboolean is_null_modem_running(BridgeApp *app);
gboolean test_null_modem_communication(BridgeApp *app);
void cleanup_devices(BridgeApp *app);

// ui.h
void create_main_window(BridgeApp *app);
gboolean update_ui_state(gpointer data);
void append_log_message(BridgeApp *app, const char *message, gboolean timestamp);

// utils.h
char* get_current_timestamp(void);
void log_message(BridgeApp *app, const char *format, ...);
gboolean update_status_timer(gpointer data);
void format_connection_time(BridgeApp *app, char *buffer, size_t buffer_size);

// callbacks.h
void connect_signals(BridgeApp *app);
void on_start_button_clicked(GtkButton *button, gpointer user_data);
void on_stop_button_clicked(GtkButton *button, gpointer user_data);
void on_test_button_clicked(GtkButton *button, gpointer user_data);
void on_clear_log_clicked(GtkButton *button, gpointer user_data);
void on_window_destroy(GtkWidget *widget, gpointer user_data);

// settings.h
void load_settings(BridgeApp *app);
void save_settings(BridgeApp *app);
void apply_appearance_settings(BridgeApp *app);

// sniffing.h
gboolean init_sniffing(BridgeApp *app);
void cleanup_sniffing(BridgeApp *app);
gboolean start_sniffing(BridgeApp *app);
void stop_sniffing(BridgeApp *app);
gboolean is_sniffing_active(BridgeApp *app);
void process_sniff_data(BridgeApp *app, const char *data, size_t len, char direction);

#endif // COMMON_H
