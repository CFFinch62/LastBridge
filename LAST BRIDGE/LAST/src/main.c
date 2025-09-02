/*
 * Main application file for LAST - Linux Advanced Serial Transceiver
 * Coordinates all modules and handles application initialization
 */

#include "common.h"
#include "serial.h"
#include "ui.h"
#include "file_ops.h"
#include "utils.h"
#include "settings.h"
#include "callbacks.h"

// Global terminal instance (defined here, declared in common.h)
SerialTerminal *g_terminal = NULL;

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    SerialTerminal terminal = {0};
    g_terminal = &terminal;

    // Initialize settings
    terminal.hex_display = FALSE;
    terminal.show_timestamps = FALSE;
    terminal.autoscroll = TRUE;
    terminal.local_echo = FALSE;
    terminal.line_ending = strdup("\r\n");
    terminal.log_file = NULL;
    terminal.log_filename = NULL;

    // Initialize appearance settings
    terminal.font_family = strdup("Monospace");
    terminal.font_size = 10;
    terminal.bg_color = strdup("#000000");
    terminal.text_color = strdup("#00FF00");
    terminal.theme_preference = strdup("system");

    // Initialize connection settings
    terminal.saved_port = NULL;
    terminal.saved_baudrate = strdup("9600");
    terminal.saved_databits = strdup("8");
    terminal.saved_parity = strdup("None");
    terminal.saved_stopbits = strdup("1");
    terminal.saved_flowcontrol = strdup("None");

    // Initialize repeat file sending
    terminal.repeat_file_sending = FALSE;
    terminal.repeat_timer_id = 0;
    terminal.repeat_filename = NULL;
    terminal.repeat_interval = 1.0;

    // Initialize serial connection
    terminal.connected = FALSE;
    terminal.thread_running = FALSE;
    terminal.serial_fd = -1;
    terminal.bytes_sent = 0;
    terminal.bytes_received = 0;

    // Load settings before creating UI
    load_settings(&terminal);

    // Create main interface
    create_main_interface(&terminal);

    // Initialize port list
    scan_all_serial_devices(GTK_COMBO_BOX_TEXT(terminal.port_combo));

    // Apply loaded settings to UI widgets BEFORE connecting signals
    apply_loaded_settings(&terminal);

    // Apply initial appearance settings
    apply_appearance_settings(&terminal);

    // Connect all signals AFTER settings are applied
    connect_signals(&terminal);

    // Start statistics update timer (every second)
    g_timeout_add(1000, update_statistics_timer, &terminal);

    // Show window
    gtk_widget_show_all(terminal.window);

    // Run main loop
    gtk_main();

    return 0;
}
