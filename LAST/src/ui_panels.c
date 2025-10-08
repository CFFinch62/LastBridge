/*
 * Panel creation for LAST - Linux Advanced Serial Terminal
 * Handles creation of connection, display, appearance, file ops, and control signal panels
 */

#include "ui.h"
#include "callbacks.h"

void create_connection_panel(SerialTerminal *terminal, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new("Connection Settings");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 2);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(frame), grid);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 5);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    int row = 0;

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Connection:"), 0, row, 1, 1);
    terminal->connection_type_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->connection_type_combo, -1, 28);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->connection_type_combo), "Serial");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->connection_type_combo), "TCP Client");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->connection_type_combo), "TCP Server");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->connection_type_combo), "UDP Client");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->connection_type_combo), "UDP Server");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->connection_type_combo), 0); // Serial
    gtk_grid_attach(GTK_GRID(grid), terminal->connection_type_combo, 1, row, 2, 1);
    row++;

    // Create frames for serial and network settings
    terminal->serial_settings_frame = gtk_frame_new("Serial Settings");
    gtk_grid_attach(GTK_GRID(grid), terminal->serial_settings_frame, 0, row, 3, 1);
    row++;

    terminal->network_settings_frame = gtk_frame_new("Network Settings");
    gtk_grid_attach(GTK_GRID(grid), terminal->network_settings_frame, 0, row, 3, 1);
    gtk_widget_set_no_show_all(terminal->network_settings_frame, TRUE); // Hidden by default
    row++;

    GtkWidget *serial_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(terminal->serial_settings_frame), serial_grid);
    gtk_container_set_border_width(GTK_CONTAINER(serial_grid), 5);
    gtk_grid_set_row_spacing(GTK_GRID(serial_grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID(serial_grid), 5);

    int serial_row = 0;

    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Port:"), 0, serial_row, 1, 1);
    terminal->port_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->port_combo, -1, 28);
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->port_combo, 1, serial_row, 1, 1);

    terminal->refresh_button = gtk_button_new_with_label("Refresh");
    gtk_widget_set_size_request(terminal->refresh_button, -1, 28);
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->refresh_button, 2, serial_row, 1, 1);
    serial_row++;

    // Baud rate
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Baud Rate:"), 0, serial_row, 1, 1);
    terminal->baudrate_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->baudrate_combo, -1, 28);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "300");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "1200");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "2400");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "4800");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "9600");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "19200");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "38400");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "57600");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "115200");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "230400");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "460800");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo), "921600");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->baudrate_combo), 4); // 9600
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->baudrate_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Data bits
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Data Bits:"), 0, serial_row, 1, 1);
    terminal->databits_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->databits_combo, -1, 28);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "6");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "7");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "8");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->databits_combo), 3); // 8
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->databits_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Parity
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Parity:"), 0, serial_row, 1, 1);
    terminal->parity_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->parity_combo, -1, 28);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "Even");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "Odd");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->parity_combo), 0); // None
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->parity_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Stop bits
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Stop Bits:"), 0, serial_row, 1, 1);
    terminal->stopbits_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->stopbits_combo, -1, 28);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo), "1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo), "2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->stopbits_combo), 0); // 1
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->stopbits_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Flow control
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Flow Control:"), 0, serial_row, 1, 1);
    terminal->flowcontrol_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->flowcontrol_combo, -1, 28);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "Hardware");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "Software");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->flowcontrol_combo), 0); // None
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->flowcontrol_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Create network settings grid
    GtkWidget *network_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(terminal->network_settings_frame), network_grid);
    gtk_container_set_border_width(GTK_CONTAINER(network_grid), 5);  // Reduced from 10 to 5
    gtk_grid_set_row_spacing(GTK_GRID(network_grid), 2);  // Reduced from 5 to 2
    gtk_grid_set_column_spacing(GTK_GRID(network_grid), 5);  // Reduced from 10 to 5

    int network_row = 0;

    // Host/IP address
    gtk_grid_attach(GTK_GRID(network_grid), gtk_label_new("Host/IP:"), 0, network_row, 1, 1);
    terminal->network_host_entry = gtk_entry_new();
    gtk_widget_set_size_request(terminal->network_host_entry, -1, 28);
    gtk_entry_set_text(GTK_ENTRY(terminal->network_host_entry), "localhost");
    gtk_entry_set_placeholder_text(GTK_ENTRY(terminal->network_host_entry), "hostname or IP address");
    gtk_grid_attach(GTK_GRID(network_grid), terminal->network_host_entry, 1, network_row, 2, 1);
    network_row++;

    // Port
    gtk_grid_attach(GTK_GRID(network_grid), gtk_label_new("Port:"), 0, network_row, 1, 1);
    terminal->network_port_entry = gtk_entry_new();
    gtk_widget_set_size_request(terminal->network_port_entry, -1, 28);
    gtk_entry_set_text(GTK_ENTRY(terminal->network_port_entry), "10110");
    gtk_entry_set_placeholder_text(GTK_ENTRY(terminal->network_port_entry), "1-65535");
    gtk_grid_attach(GTK_GRID(network_grid), terminal->network_port_entry, 1, network_row, 2, 1);
    network_row++;

    // Connection buttons (shared between serial and network)
    terminal->connect_button = gtk_button_new_with_label("Connect");
    gtk_widget_set_size_request(terminal->connect_button, -1, 28);
    gtk_grid_attach(GTK_GRID(grid), terminal->connect_button, 0, row, 1, 1);

    terminal->disconnect_button = gtk_button_new_with_label("Disconnect");
    gtk_widget_set_size_request(terminal->disconnect_button, -1, 28);
    gtk_widget_set_sensitive(terminal->disconnect_button, FALSE);
    gtk_grid_attach(GTK_GRID(grid), terminal->disconnect_button, 1, row, 2, 1);
}

void create_display_options_panel(SerialTerminal *terminal, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new("Display Options");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    terminal->hex_display_check = gtk_check_button_new_with_label("Hex Display");
    gtk_box_pack_start(GTK_BOX(vbox), terminal->hex_display_check, FALSE, FALSE, 0);

    // Hex bytes per line control
    GtkWidget *hex_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hex_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hex_hbox), gtk_label_new("Hex Bytes/Line:"), FALSE, FALSE, 0);
    terminal->hex_bytes_per_line_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "Auto (CR+LF)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "8");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "16");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "32");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "64");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "128");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "256");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->hex_bytes_per_line_combo), "512");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo), 0); // Auto (CR+LF)
    gtk_box_pack_start(GTK_BOX(hex_hbox), terminal->hex_bytes_per_line_combo, TRUE, TRUE, 0);

    terminal->timestamp_check = gtk_check_button_new_with_label("Show Timestamps");
    gtk_box_pack_start(GTK_BOX(vbox), terminal->timestamp_check, FALSE, FALSE, 0);

    terminal->autoscroll_check = gtk_check_button_new_with_label("Auto Scroll");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->autoscroll_check), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), terminal->autoscroll_check, FALSE, FALSE, 0);

    terminal->local_echo_check = gtk_check_button_new_with_label("Local Echo");
    gtk_box_pack_start(GTK_BOX(vbox), terminal->local_echo_check, FALSE, FALSE, 0);

    // Line ending
    GtkWidget *line_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), line_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(line_hbox), gtk_label_new("Line Ending:"), FALSE, FALSE, 0);
    terminal->line_ending_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->line_ending_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->line_ending_combo), "CR");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->line_ending_combo), "LF");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->line_ending_combo), "CR+LF");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->line_ending_combo), 3); // CR+LF
    gtk_box_pack_start(GTK_BOX(line_hbox), terminal->line_ending_combo, TRUE, TRUE, 0);
}

void create_appearance_panel(SerialTerminal *terminal, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new("Appearance");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Theme selection
    GtkWidget *theme_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), theme_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(theme_hbox), gtk_label_new("Theme:"), FALSE, FALSE, 0);
    terminal->theme_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo), "System");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo), "Light");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo), "Dark");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->theme_combo), 0); // System default
    gtk_box_pack_start(GTK_BOX(theme_hbox), terminal->theme_combo, TRUE, TRUE, 0);

    // Font selection
    GtkWidget *font_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), font_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(font_hbox), gtk_label_new("Font:"), FALSE, FALSE, 0);
    terminal->font_button = gtk_font_button_new();
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(terminal->font_button), "Monospace 10");
    gtk_box_pack_start(GTK_BOX(font_hbox), terminal->font_button, TRUE, TRUE, 0);

    // Color selection
    GtkWidget *color_grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), color_grid, FALSE, FALSE, 0);
    gtk_grid_set_row_spacing(GTK_GRID(color_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(color_grid), 10);

    // Background color
    gtk_grid_attach(GTK_GRID(color_grid), gtk_label_new("Background:"), 0, 0, 1, 1);
    terminal->bg_color_button = gtk_color_button_new();
    GdkRGBA bg_color = {0.0, 0.0, 0.0, 1.0}; // Black default
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(terminal->bg_color_button), &bg_color);
    gtk_grid_attach(GTK_GRID(color_grid), terminal->bg_color_button, 1, 0, 1, 1);

    // Text color
    gtk_grid_attach(GTK_GRID(color_grid), gtk_label_new("Text:"), 0, 1, 1, 1);
    terminal->text_color_button = gtk_color_button_new();
    GdkRGBA text_color = {0.0, 1.0, 0.0, 1.0}; // Green default
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(terminal->text_color_button), &text_color);
    gtk_grid_attach(GTK_GRID(color_grid), terminal->text_color_button, 1, 1, 1, 1);
}

void create_file_operations_panel(SerialTerminal *terminal, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new("File Operations");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 2);  // Reduced from 5 to 2

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);  // Reduced from 5 to 3
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);  // Reduced from 10 to 5

    // Send file section
    GtkWidget *send_file_frame = gtk_frame_new("Send File");
    gtk_box_pack_start(GTK_BOX(vbox), send_file_frame, FALSE, FALSE, 0);

    GtkWidget *send_file_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);  // Reduced from 3 to 2
    gtk_container_add(GTK_CONTAINER(send_file_frame), send_file_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(send_file_vbox), 3);  // Reduced from 5 to 3

    terminal->send_file_button = gtk_button_new_with_label("Send File...");
    gtk_widget_set_size_request(terminal->send_file_button, -1, 28);
    gtk_widget_set_sensitive(terminal->send_file_button, FALSE);
    gtk_box_pack_start(GTK_BOX(send_file_vbox), terminal->send_file_button, FALSE, FALSE, 0);

    // Repeat and Lines controls
    GtkWidget *options_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);  // Reduced from 10 to 5
    gtk_box_pack_start(GTK_BOX(send_file_vbox), options_hbox, FALSE, FALSE, 0);

    terminal->send_file_repeat_check = gtk_check_button_new_with_label("Repeat");
    gtk_box_pack_start(GTK_BOX(options_hbox), terminal->send_file_repeat_check, FALSE, FALSE, 0);

    terminal->send_file_lines_check = gtk_check_button_new_with_label("Lines");
    gtk_box_pack_start(GTK_BOX(options_hbox), terminal->send_file_lines_check, FALSE, FALSE, 0);

    GtkWidget *interval_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);  // Reduced from 5 to 3
    gtk_box_pack_start(GTK_BOX(send_file_vbox), interval_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(interval_hbox), gtk_label_new("Interval:"), FALSE, FALSE, 0);

    terminal->send_file_interval_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(terminal->send_file_interval_combo, -1, 28);
    gtk_box_pack_start(GTK_BOX(interval_hbox), terminal->send_file_interval_combo, TRUE, TRUE, 0);

    terminal->send_file_interval_label = gtk_label_new("sec");
    gtk_box_pack_start(GTK_BOX(interval_hbox), terminal->send_file_interval_label, FALSE, FALSE, 0);

    // Initialize with repeat mode intervals
    populate_interval_dropdown_for_repeat(terminal);

    terminal->send_file_stop_button = gtk_button_new_with_label("Stop Repeat");
    gtk_widget_set_size_request(terminal->send_file_stop_button, -1, 28);
    gtk_widget_set_sensitive(terminal->send_file_stop_button, FALSE);
    gtk_box_pack_start(GTK_BOX(send_file_vbox), terminal->send_file_stop_button, FALSE, FALSE, 0);

    // Logging operations
    terminal->log_file_button = gtk_toggle_button_new_with_label("Log to File");
    gtk_widget_set_size_request(terminal->log_file_button, -1, 28);
    gtk_box_pack_start(GTK_BOX(vbox), terminal->log_file_button, FALSE, FALSE, 0);

    terminal->log_file_entry = gtk_entry_new();
    gtk_widget_set_size_request(terminal->log_file_entry, -1, 28);
    gtk_entry_set_placeholder_text(GTK_ENTRY(terminal->log_file_entry), "Log file path...");
    gtk_box_pack_start(GTK_BOX(vbox), terminal->log_file_entry, FALSE, FALSE, 0);
}

void create_control_signals_panel(SerialTerminal *terminal, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new("Control Signals");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 2);  // Reduced from 5 to 2

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);  // Reduced from 5 to 3
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);  // Reduced from 10 to 5

    GtkWidget *signals_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);  // Reduced from 10 to 5
    gtk_box_pack_start(GTK_BOX(vbox), signals_hbox, FALSE, FALSE, 0);

    terminal->dtr_check = gtk_check_button_new_with_label("DTR");
    gtk_box_pack_start(GTK_BOX(signals_hbox), terminal->dtr_check, FALSE, FALSE, 0);

    terminal->rts_check = gtk_check_button_new_with_label("RTS");
    gtk_box_pack_start(GTK_BOX(signals_hbox), terminal->rts_check, FALSE, FALSE, 0);

    terminal->break_button = gtk_button_new_with_label("Send Break");
    gtk_widget_set_size_request(terminal->break_button, -1, 28);
    gtk_widget_set_sensitive(terminal->break_button, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), terminal->break_button, FALSE, FALSE, 0);
}

static GtkWidget* create_indicator(const char *label, const char *color) {
    GtkWidget *indicator = gtk_label_new(label);
    gtk_widget_set_size_request(indicator, 30, 20);
    gtk_label_set_justify(GTK_LABEL(indicator), GTK_JUSTIFY_CENTER);

    // Set initial styling
    char css[256];
    snprintf(css, sizeof(css),
        "label { background-color: %s; color: white; font-weight: bold; "
        "border: 1px solid #333; border-radius: 3px; font-size: 9px; }", color);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(indicator);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    return indicator;
}

void create_signal_indicators(SerialTerminal *terminal, GtkWidget *parent) {
    // Create a frame for the indicators
    GtkWidget *indicators_frame = gtk_frame_new("Signal Status");
    gtk_box_pack_start(GTK_BOX(parent), indicators_frame, FALSE, FALSE, 0);

    // Create horizontal box for indicators
    GtkWidget *indicators_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_container_add(GTK_CONTAINER(indicators_frame), indicators_hbox);
    gtk_container_set_border_width(GTK_CONTAINER(indicators_hbox), 3);

    // Create TX/RX activity indicators (gray when inactive, yellow when active)
    terminal->tx_indicator = create_indicator("TX", "#666666");  // Gray when inactive
    gtk_box_pack_start(GTK_BOX(indicators_hbox), terminal->tx_indicator, FALSE, FALSE, 0);

    terminal->rx_indicator = create_indicator("RX", "#666666");  // Gray when inactive
    gtk_box_pack_start(GTK_BOX(indicators_hbox), terminal->rx_indicator, FALSE, FALSE, 0);

    // Create control signal indicators (red when inactive, green when active)
    terminal->cts_indicator = create_indicator("CTS", "#CC0000");  // Red when inactive
    gtk_box_pack_start(GTK_BOX(indicators_hbox), terminal->cts_indicator, FALSE, FALSE, 0);

    terminal->rts_indicator = create_indicator("RTS", "#CC0000");  // Red when inactive
    gtk_box_pack_start(GTK_BOX(indicators_hbox), terminal->rts_indicator, FALSE, FALSE, 0);

    terminal->dtr_indicator = create_indicator("DTR", "#CC0000");  // Red when inactive
    gtk_box_pack_start(GTK_BOX(indicators_hbox), terminal->dtr_indicator, FALSE, FALSE, 0);

    terminal->dsr_indicator = create_indicator("DSR", "#CC0000");  // Red when inactive
    gtk_box_pack_start(GTK_BOX(indicators_hbox), terminal->dsr_indicator, FALSE, FALSE, 0);
}

