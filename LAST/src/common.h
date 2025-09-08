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
#include <glib.h>
#include <pthread.h>
#include <time.h>

// Define CRTSCTS if not available
#ifndef CRTSCTS
#define CRTSCTS 020000000000
#endif

// Main application data structure
typedef struct {
    // Main window and layout
    GtkWidget *window;
    GtkWidget *main_paned;
    GtkWidget *notebook;

    // Menu bar
    GtkWidget *menu_bar;
    GtkWidget *file_menu;
    GtkWidget *tools_menu;
    GtkWidget *help_menu;

    // Connection settings
    GtkWidget *port_combo;
    GtkWidget *baudrate_combo;
    GtkWidget *databits_combo;
    GtkWidget *parity_combo;
    GtkWidget *stopbits_combo;
    GtkWidget *flowcontrol_combo;
    GtkWidget *connect_button;
    GtkWidget *disconnect_button;
    GtkWidget *refresh_button;

    // Data handling
    GtkWidget *receive_text;
    GtkWidget *send_entry;
    GtkWidget *send_button;
    GtkWidget *clear_button;
    GtkWidget *save_button;

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
    GtkWidget *send_file_interval_combo;
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

    // Serial connection
    int serial_fd;
    gboolean connected;
    pthread_t read_thread;
    gboolean thread_running;

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
    char *saved_port;
    char *saved_baudrate;
    char *saved_databits;
    char *saved_parity;
    char *saved_stopbits;
    char *saved_flowcontrol;

    // File logging
    FILE *log_file;
    char *log_filename;

    // Repeat file sending
    gboolean repeat_file_sending;
    guint repeat_timer_id;
    char *repeat_filename;
    double repeat_interval;
} SerialTerminal;

// Global terminal instance (declared here, defined in main.c)
extern SerialTerminal *g_terminal;

// Data structure for idle callback
typedef struct {
    char *text;
    gboolean is_received;
} IdleCallbackData;

#endif // COMMON_H
