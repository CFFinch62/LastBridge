#ifndef COMMON_H
#define COMMON_H

/*
 * Common header file for LAST - Linux Advanced Serial Transceiver
 * Contains shared includes, constants, and data structures
 */

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <glib.h>
#include <pthread.h>
#include <time.h>

// Define CRTSCTS if not available
#ifndef CRTSCTS
#define CRTSCTS 020000000000
#endif

// Macro button constants
#define MAX_MACRO_BUTTONS 16
#define MAX_MACRO_LABEL_LENGTH 32
#define MAX_MACRO_COMMAND_LENGTH 256

// Network connection constants
#define MAX_HOSTNAME_LENGTH 256
#define MAX_PORT_LENGTH 8
#define DEFAULT_NETWORK_PORT 10110  // Common NMEA 0183 over TCP port

// Connection types
typedef enum {
    CONNECTION_TYPE_SERIAL = 0,
    CONNECTION_TYPE_TCP_CLIENT,
    CONNECTION_TYPE_TCP_SERVER,
    CONNECTION_TYPE_UDP_CLIENT,
    CONNECTION_TYPE_UDP_SERVER
} ConnectionType;

// Main application data structure
typedef struct {
    // Main window and layout
    GtkWidget *window;
    GtkWidget *main_hbox;

    // Menu bar
    GtkWidget *menu_bar;
    GtkWidget *file_menu;
    GtkWidget *view_menu;
    GtkWidget *tools_menu;
    GtkWidget *macros_menu;
    GtkWidget *help_menu;

    // Connection settings
    GtkWidget *connection_type_combo;
    GtkWidget *port_combo;
    GtkWidget *baudrate_combo;
    GtkWidget *databits_combo;
    GtkWidget *parity_combo;
    GtkWidget *stopbits_combo;
    GtkWidget *flowcontrol_combo;
    GtkWidget *connect_button;
    GtkWidget *disconnect_button;
    GtkWidget *refresh_button;

    // Network connection settings
    GtkWidget *network_host_entry;
    GtkWidget *network_port_entry;
    GtkWidget *network_settings_frame;
    GtkWidget *serial_settings_frame;

    // Data handling
    GtkWidget *receive_text;
    GtkWidget *hex_text;  // New hex display text view
    GtkWidget *hex_frame; // Frame containing hex display area
    GtkWidget *hex_scrolled; // Scrolled window for hex display
    GtkWidget *send_entry;
    GtkWidget *send_button;
    GtkWidget *clear_button;
    GtkWidget *save_button;

    // Signal line indicators
    GtkWidget *tx_indicator;
    GtkWidget *rx_indicator;
    GtkWidget *cts_indicator;
    GtkWidget *rts_indicator;
    GtkWidget *dtr_indicator;
    GtkWidget *dsr_indicator;

    // Display options
    GtkWidget *hex_display_check;
    GtkWidget *hex_bytes_per_line_combo;
    GtkWidget *timestamp_check;
    GtkWidget *autoscroll_check;
    GtkWidget *local_echo_check;
    GtkWidget *line_ending_combo;

    // File operations
    GtkWidget *send_file_button;
    GtkWidget *send_file_repeat_check;
    GtkWidget *send_file_lines_check;
    GtkWidget *send_file_interval_combo;
    GtkWidget *send_file_interval_label;
    GtkWidget *send_file_stop_button;
    GtkWidget *log_file_button;
    GtkWidget *log_file_entry;

    // Control signals
    GtkWidget *dtr_check;
    GtkWidget *rts_check;
    GtkWidget *break_button;

    // Status and statistics
    GtkWidget *status_label;
    GtkWidget *stats_label;
    GtkWidget *connection_time_label;

    // Appearance customization widgets
    GtkWidget *font_button;
    GtkWidget *bg_color_button;
    GtkWidget *text_color_button;
    GtkWidget *theme_combo;

    // Connection management
    ConnectionType connection_type;
    int connection_fd;  // Unified file descriptor for serial or network
    gboolean connected;
    pthread_t read_thread;
    gboolean thread_running;

    // Network connection details
    char network_host[MAX_HOSTNAME_LENGTH];
    char network_port[MAX_PORT_LENGTH];
    int server_fd;  // For server modes (TCP/UDP server)
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    // Statistics
    unsigned long bytes_sent;
    unsigned long bytes_received;
    time_t connection_start_time;

    // Settings
    gboolean hex_display;
    int hex_bytes_per_line;
    gboolean show_timestamps;
    gboolean autoscroll;
    gboolean local_echo;
    char *line_ending;

    // Appearance settings
    char *font_family;
    int font_size;
    char *bg_color;
    char *text_color;
    char *theme_preference; // "dark", "light", or "system"

    // Connection settings (for persistence)
    char *saved_connection_type;
    char *saved_port;
    char *saved_baudrate;
    char *saved_databits;
    char *saved_parity;
    char *saved_stopbits;
    char *saved_flowcontrol;
    char *saved_network_host;
    char *saved_network_port;

    // Scripting engine
    void *lua_state;                    // lua_State* (void* to avoid including lua.h here)
    gboolean scripting_enabled;
    char *script_content;

    // Script UI widgets
    GtkWidget *script_window;
    GtkWidget *script_text_view;
    GtkWidget *script_load_button;
    GtkWidget *script_save_button;
    GtkWidget *script_clear_button;
    GtkWidget *script_enable_check;
    GtkWidget *script_test_button;

    // File logging
    FILE *log_file;
    char *log_filename;

    // Repeat file sending
    gboolean repeat_file_sending;
    guint repeat_timer_id;
    char *repeat_filename;
    double repeat_interval;

    // Line-by-line file sending
    gboolean line_by_line_sending;
    gboolean line_by_line_mode;
    guint line_by_line_timer_id;
    FILE *line_by_line_file;
    int line_by_line_delay_ms;
    int current_line_number;

    // Signal line status and activity tracking
    guint signal_update_timer_id;
    gboolean tx_active;
    gboolean rx_active;
    time_t tx_last_activity;
    time_t rx_last_activity;

    // Macro buttons
    GtkWidget *macro_panel;
    GtkWidget *macro_buttons[MAX_MACRO_BUTTONS];
    char macro_labels[MAX_MACRO_BUTTONS][MAX_MACRO_LABEL_LENGTH];
    char macro_commands[MAX_MACRO_BUTTONS][MAX_MACRO_COMMAND_LENGTH];
    gboolean macro_panel_visible;
} SerialTerminal;

// Global terminal instance (declared here, defined in main.c)
extern SerialTerminal *g_terminal;

// Data structure for idle callback
typedef struct {
    char *text;
    gboolean is_received;
} IdleCallbackData;

// Data structure for dual display idle callback
typedef struct {
    char *text_data;
    char *hex_data;
    gboolean show_hex;
} DualDisplayData;

#endif // COMMON_H
