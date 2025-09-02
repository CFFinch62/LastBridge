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
#define _POSIX_C_SOURCE 200809L

// GTK includes
#include <gtk/gtk.h>

// Application constants
#define MAX_PATH_LENGTH 256
#define MAX_LOG_LENGTH 1024
#define DEFAULT_DEVICE1 "/tmp/ttyV0"
#define DEFAULT_DEVICE2 "/tmp/ttyV1"
#define SOCAT_CHECK_INTERVAL 1000  // milliseconds
#define DEVICE_CREATION_TIMEOUT 5000  // milliseconds

// Application states
typedef enum {
    BRIDGE_STATE_STOPPED,
    BRIDGE_STATE_STARTING,
    BRIDGE_STATE_RUNNING,
    BRIDGE_STATE_STOPPING,
    BRIDGE_STATE_ERROR
} BridgeState;

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

#endif // COMMON_H
