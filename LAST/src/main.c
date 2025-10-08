/*
 * Main application file for LAST - Linux Advanced Serial Transceiver
 * Coordinates all modules and handles application initialization
 */

#include "common.h"
#include "serial.h"
#include "network.h"
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
    terminal.hex_display = TRUE;  // Default to showing hex display
    terminal.hex_bytes_per_line = 0;  // 0 = Auto mode (CR+LF detection)
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
    terminal.connection_type = CONNECTION_TYPE_SERIAL;
    terminal.connection_fd = -1;
    terminal.server_fd = -1;
    terminal.client_addr_len = 0;
    strncpy(terminal.network_host, "localhost", MAX_HOSTNAME_LENGTH - 1);
    terminal.network_host[MAX_HOSTNAME_LENGTH - 1] = '\0';
    strncpy(terminal.network_port, "10110", MAX_PORT_LENGTH - 1);
    terminal.network_port[MAX_PORT_LENGTH - 1] = '\0';

    terminal.saved_connection_type = strdup("Serial");
    terminal.saved_port = NULL;
    terminal.saved_baudrate = strdup("9600");
    terminal.saved_databits = strdup("8");
    terminal.saved_parity = strdup("None");
    terminal.saved_stopbits = strdup("1");
    terminal.saved_flowcontrol = strdup("None");
    terminal.saved_network_host = strdup("localhost");
    terminal.saved_network_port = strdup("10110");

    // Initialize repeat file sending
    terminal.repeat_file_sending = FALSE;
    terminal.repeat_timer_id = 0;
    terminal.repeat_filename = NULL;
    terminal.repeat_interval = 1.0;

    // Initialize line-by-line file sending
    terminal.line_by_line_sending = FALSE;
    terminal.line_by_line_mode = FALSE;
    terminal.line_by_line_timer_id = 0;
    terminal.line_by_line_file = NULL;
    terminal.line_by_line_delay_ms = 100;  // Default 100ms
    terminal.current_line_number = 0;

    // Initialize signal line monitoring
    terminal.signal_update_timer_id = 0;
    terminal.tx_active = FALSE;
    terminal.rx_active = FALSE;
    terminal.tx_last_activity = 0;
    terminal.rx_last_activity = 0;

    // Initialize macro buttons
    terminal.macro_panel = NULL;
    terminal.macro_panel_visible = TRUE; // Default to visible
    for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
        terminal.macro_buttons[i] = NULL;
        snprintf(terminal.macro_labels[i], MAX_MACRO_LABEL_LENGTH, "Macro %d", i + 1);
        terminal.macro_commands[i][0] = '\0'; // Empty command initially
    }

    // Initialize connection state
    terminal.connected = FALSE;
    terminal.thread_running = FALSE;
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
