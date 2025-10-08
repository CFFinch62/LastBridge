/*
 * User Interface module for LAST - Linux Advanced Serial Terminal
 * Handles GTK UI creation and layout
 */

#include "ui.h"
#include "callbacks.h"
#include "settings.h"

void create_main_interface(SerialTerminal *terminal) {
    // Create main window - adjust for total height including decorations to be max 720px
    terminal->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(terminal->window), "LAST - Linux Advanced Serial Terminal");

    // Set window size to exactly 720px total height for laptop compatibility
    gtk_window_set_default_size(GTK_WINDOW(terminal->window), 1280, 720);

    // Set minimum window size to prevent resizing below usable dimensions
    gtk_widget_set_size_request(terminal->window, 1280, 720);

    // Try to force the window to be exactly 720px high
    gtk_window_resize(GTK_WINDOW(terminal->window), 1280, 720);

    // Ensure window is resizable for larger screens and has maximize button
    gtk_window_set_resizable(GTK_WINDOW(terminal->window), TRUE);

    // Enable maximize button by setting window type hint
    gtk_window_set_type_hint(GTK_WINDOW(terminal->window), GDK_WINDOW_TYPE_HINT_NORMAL);

    // Set geometry hints to control window sizing
    GdkGeometry geometry;
    geometry.min_width = 1280;
    geometry.min_height = 720;
    geometry.max_width = -1;  // No maximum width
    geometry.max_height = -1; // No maximum height
    gtk_window_set_geometry_hints(GTK_WINDOW(terminal->window), NULL, &geometry,
                                  GDK_HINT_MIN_SIZE);

    // Center the window on screen
    gtk_window_set_position(GTK_WINDOW(terminal->window), GTK_WIN_POS_CENTER);

    // Set window icon - try multiple approaches
    GdkPixbuf *icon = NULL;
    GError *error = NULL;

    // Try loading PNG icon first (smaller, better for window icons)
    const char *icon_paths[] = {
        "last-icon.png",                           // Current directory (PNG)
        "last-icon.jpg",                           // Current directory (JPG fallback)
        "/usr/local/share/pixmaps/last-icon.png",  // System location (PNG)
        "/usr/local/share/pixmaps/last-icon.jpg",  // System location (JPG fallback)
        "/usr/share/pixmaps/last-icon.png",        // Alternative system location (PNG)
        "/usr/share/pixmaps/last-icon.jpg",        // Alternative system location (JPG fallback)
        NULL
    };

    for (int i = 0; icon_paths[i] != NULL && !icon; i++) {
        error = NULL;
        icon = gdk_pixbuf_new_from_file(icon_paths[i], &error);
        if (icon) {
            // Scale icon to appropriate size for window decoration
            GdkPixbuf *scaled_icon = gdk_pixbuf_scale_simple(icon, 48, 48, GDK_INTERP_BILINEAR);
            if (scaled_icon) {
                gtk_window_set_icon(GTK_WINDOW(terminal->window), scaled_icon);
                g_object_unref(scaled_icon);
            }
            g_object_unref(icon);
            break;
        } else if (error) {
            g_error_free(error);
        }
    }

    // Create main container
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(terminal->window), main_vbox);

    // Create menu bar
    create_menu_bar(terminal, main_vbox);

    // Create main horizontal layout with two columns - reduce spacing to save height
    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3); // Reduced from 5 to 3
    gtk_box_pack_start(GTK_BOX(main_vbox), main_hbox, TRUE, TRUE, 2); // Reduced from 5 to 2

    // Create left panel (Connection, Control Signals, File Operations)
    GtkWidget *left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3); // Reduced spacing from 5 to 3
    // Set minimum width for left panel but allow it to be responsive
    gtk_widget_set_size_request(left_panel, 300, -1); // Reduced from 320 to 300
    gtk_box_pack_start(GTK_BOX(main_hbox), left_panel, FALSE, FALSE, 15); // Reduced margin from 20 to 15

    // Create connection panel (upper left)
    create_connection_panel(terminal, left_panel);

    // Create control signals panel (middle left)
    create_control_signals_panel(terminal, left_panel);

    // Create file operations panel (bottom left)
    create_file_operations_panel(terminal, left_panel);

    // Create macro panel (middle column)
    terminal->macro_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_widget_set_size_request(terminal->macro_panel, 200, -1); // Fixed width for macro buttons
    gtk_box_pack_start(GTK_BOX(main_hbox), terminal->macro_panel, FALSE, FALSE, 10);
    create_macro_panel(terminal, terminal->macro_panel);

    // Create center panel (Data area and send controls) - responsive width
    GtkWidget *center_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3); // Reduced spacing from 5 to 3
    // Remove fixed width constraint to allow proper expansion
    // gtk_widget_set_size_request(center_panel, 800, -1); // Removed fixed width
    gtk_box_pack_start(GTK_BOX(main_hbox), center_panel, TRUE, TRUE, 15); // Reduced margin from 20 to 15

    // Create data area in center
    create_data_area(terminal, center_panel);

    // Create hidden appearance and display options panels for menu dialogs
    // These widgets are needed for the menu callbacks but not displayed in the main UI
    GtkWidget *hidden_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    create_appearance_panel(terminal, hidden_container);
    create_display_options_panel(terminal, hidden_container);
    // Don't add hidden_container to any visible parent

    // Create status bar
    GtkWidget *status_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(main_vbox), status_hbox, FALSE, FALSE, 0);

    terminal->status_label = gtk_label_new("Disconnected");
    gtk_widget_set_halign(terminal->status_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(status_hbox), terminal->status_label, TRUE, TRUE, 5);

    terminal->stats_label = gtk_label_new("Sent: 0 | Received: 0 | Time: 00:00:00");
    gtk_box_pack_start(GTK_BOX(status_hbox), terminal->stats_label, FALSE, FALSE, 5);
}

void create_connection_panel(SerialTerminal *terminal, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new("Connection Settings");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(frame), grid);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    int row = 0;

    // Connection type selection
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Connection:"), 0, row, 1, 1);
    terminal->connection_type_combo = gtk_combo_box_text_new();
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

    // Create serial settings grid
    GtkWidget *serial_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(terminal->serial_settings_frame), serial_grid);
    gtk_container_set_border_width(GTK_CONTAINER(serial_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(serial_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(serial_grid), 10);

    int serial_row = 0;

    // Port selection
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Port:"), 0, serial_row, 1, 1);
    terminal->port_combo = gtk_combo_box_text_new();
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->port_combo, 1, serial_row, 1, 1);

    terminal->refresh_button = gtk_button_new_with_label("Refresh");
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->refresh_button, 2, serial_row, 1, 1);
    serial_row++;

    // Baud rate
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Baud Rate:"), 0, serial_row, 1, 1);
    terminal->baudrate_combo = gtk_combo_box_text_new();
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
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "Even");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "Odd");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->parity_combo), 0); // None
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->parity_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Stop bits
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Stop Bits:"), 0, serial_row, 1, 1);
    terminal->stopbits_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo), "1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo), "2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->stopbits_combo), 0); // 1
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->stopbits_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Flow control
    gtk_grid_attach(GTK_GRID(serial_grid), gtk_label_new("Flow Control:"), 0, serial_row, 1, 1);
    terminal->flowcontrol_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "Hardware");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "Software");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->flowcontrol_combo), 0); // None
    gtk_grid_attach(GTK_GRID(serial_grid), terminal->flowcontrol_combo, 1, serial_row, 2, 1);
    serial_row++;

    // Create network settings grid
    GtkWidget *network_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(terminal->network_settings_frame), network_grid);
    gtk_container_set_border_width(GTK_CONTAINER(network_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(network_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(network_grid), 10);

    int network_row = 0;

    // Host/IP address
    gtk_grid_attach(GTK_GRID(network_grid), gtk_label_new("Host/IP:"), 0, network_row, 1, 1);
    terminal->network_host_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(terminal->network_host_entry), "localhost");
    gtk_entry_set_placeholder_text(GTK_ENTRY(terminal->network_host_entry), "hostname or IP address");
    gtk_grid_attach(GTK_GRID(network_grid), terminal->network_host_entry, 1, network_row, 2, 1);
    network_row++;

    // Port
    gtk_grid_attach(GTK_GRID(network_grid), gtk_label_new("Port:"), 0, network_row, 1, 1);
    terminal->network_port_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(terminal->network_port_entry), "10110");
    gtk_entry_set_placeholder_text(GTK_ENTRY(terminal->network_port_entry), "1-65535");
    gtk_grid_attach(GTK_GRID(network_grid), terminal->network_port_entry, 1, network_row, 2, 1);
    network_row++;

    // Connection buttons (shared between serial and network)
    terminal->connect_button = gtk_button_new_with_label("Connect");
    gtk_grid_attach(GTK_GRID(grid), terminal->connect_button, 0, row, 1, 1);

    terminal->disconnect_button = gtk_button_new_with_label("Disconnect");
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
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Send file section
    GtkWidget *send_file_frame = gtk_frame_new("Send File");
    gtk_box_pack_start(GTK_BOX(vbox), send_file_frame, FALSE, FALSE, 0);

    GtkWidget *send_file_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_add(GTK_CONTAINER(send_file_frame), send_file_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(send_file_vbox), 5);

    terminal->send_file_button = gtk_button_new_with_label("Send File...");
    gtk_widget_set_sensitive(terminal->send_file_button, FALSE);
    gtk_box_pack_start(GTK_BOX(send_file_vbox), terminal->send_file_button, FALSE, FALSE, 0);

    // Repeat and Lines controls
    GtkWidget *options_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(send_file_vbox), options_hbox, FALSE, FALSE, 0);

    terminal->send_file_repeat_check = gtk_check_button_new_with_label("Repeat");
    gtk_box_pack_start(GTK_BOX(options_hbox), terminal->send_file_repeat_check, FALSE, FALSE, 0);

    terminal->send_file_lines_check = gtk_check_button_new_with_label("Lines");
    gtk_box_pack_start(GTK_BOX(options_hbox), terminal->send_file_lines_check, FALSE, FALSE, 0);

    GtkWidget *interval_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(send_file_vbox), interval_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(interval_hbox), gtk_label_new("Interval:"), FALSE, FALSE, 0);

    terminal->send_file_interval_combo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(interval_hbox), terminal->send_file_interval_combo, TRUE, TRUE, 0);

    terminal->send_file_interval_label = gtk_label_new("sec");
    gtk_box_pack_start(GTK_BOX(interval_hbox), terminal->send_file_interval_label, FALSE, FALSE, 0);

    // Initialize with repeat mode intervals
    populate_interval_dropdown_for_repeat(terminal);

    terminal->send_file_stop_button = gtk_button_new_with_label("Stop Repeat");
    gtk_widget_set_sensitive(terminal->send_file_stop_button, FALSE);
    gtk_box_pack_start(GTK_BOX(send_file_vbox), terminal->send_file_stop_button, FALSE, FALSE, 0);

    // Logging operations
    terminal->log_file_button = gtk_toggle_button_new_with_label("Log to File");
    gtk_box_pack_start(GTK_BOX(vbox), terminal->log_file_button, FALSE, FALSE, 0);

    terminal->log_file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(terminal->log_file_entry), "Log file path...");
    gtk_box_pack_start(GTK_BOX(vbox), terminal->log_file_entry, FALSE, FALSE, 0);
}

void create_control_signals_panel(SerialTerminal *terminal, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new("Control Signals");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    GtkWidget *signals_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), signals_hbox, FALSE, FALSE, 0);

    terminal->dtr_check = gtk_check_button_new_with_label("DTR");
    gtk_box_pack_start(GTK_BOX(signals_hbox), terminal->dtr_check, FALSE, FALSE, 0);

    terminal->rts_check = gtk_check_button_new_with_label("RTS");
    gtk_box_pack_start(GTK_BOX(signals_hbox), terminal->rts_check, FALSE, FALSE, 0);

    terminal->break_button = gtk_button_new_with_label("Send Break");
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

void create_data_area(SerialTerminal *terminal, GtkWidget *parent) {
    // Receive area - make it less tall
    GtkWidget *receive_frame = gtk_frame_new("Received Data");
    gtk_box_pack_start(GTK_BOX(parent), receive_frame, TRUE, TRUE, 0);

    GtkWidget *receive_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(receive_frame), receive_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(receive_vbox), 5);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    // Enable both vertical and horizontal scroll bars
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    // Set height for receive area - default to half height since hex display is shown by default
    gtk_widget_set_size_request(scrolled, -1, 120); // Half height when hex is shown
    gtk_box_pack_start(GTK_BOX(receive_vbox), scrolled, TRUE, TRUE, 0);

    terminal->receive_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(terminal->receive_text), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(terminal->receive_text), TRUE);
    // Disable text wrapping to enable horizontal scrolling
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(terminal->receive_text), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(scrolled), terminal->receive_text);

    // Create hex display area (shown by default)
    terminal->hex_frame = gtk_frame_new("Hex Data");
    gtk_box_pack_start(GTK_BOX(parent), terminal->hex_frame, TRUE, TRUE, 0);
    // Hex display is shown by default, but can be hidden via toggle

    GtkWidget *hex_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(terminal->hex_frame), hex_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(hex_vbox), 5);

    terminal->hex_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(terminal->hex_scrolled),
                                   GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    // Set height for hex area - will be shown when hex display is enabled
    gtk_widget_set_size_request(terminal->hex_scrolled, -1, 120); // Half the original height
    gtk_box_pack_start(GTK_BOX(hex_vbox), terminal->hex_scrolled, TRUE, TRUE, 0);

    terminal->hex_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(terminal->hex_text), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(terminal->hex_text), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(terminal->hex_text), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(terminal->hex_scrolled), terminal->hex_text);

    // Realize the hex text widget so it can receive styling even when hidden
    gtk_widget_realize(terminal->hex_text);

    // Receive controls
    GtkWidget *receive_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(receive_vbox), receive_controls, FALSE, FALSE, 0);

    terminal->clear_button = gtk_button_new_with_label("Clear");
    gtk_box_pack_start(GTK_BOX(receive_controls), terminal->clear_button, FALSE, FALSE, 0);

    terminal->save_button = gtk_button_new_with_label("Save Received Data...");
    gtk_box_pack_start(GTK_BOX(receive_controls), terminal->save_button, FALSE, FALSE, 0);

    // Add spacer to push indicators to the right
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(receive_controls), spacer, TRUE, TRUE, 0);

    // Create signal line indicators
    create_signal_indicators(terminal, receive_controls);

    // Send area - keep in center
    GtkWidget *send_frame = gtk_frame_new("Send Data");
    gtk_box_pack_start(GTK_BOX(parent), send_frame, FALSE, FALSE, 0);

    GtkWidget *send_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(send_frame), send_hbox);
    gtk_container_set_border_width(GTK_CONTAINER(send_hbox), 5);

    terminal->send_entry = gtk_entry_new();
    gtk_widget_set_sensitive(terminal->send_entry, FALSE);
    gtk_box_pack_start(GTK_BOX(send_hbox), terminal->send_entry, TRUE, TRUE, 0);

    terminal->send_button = gtk_button_new_with_label("Send");
    gtk_widget_set_sensitive(terminal->send_button, FALSE);
    gtk_box_pack_start(GTK_BOX(send_hbox), terminal->send_button, FALSE, FALSE, 0);
}

void apply_appearance_settings(SerialTerminal *terminal) {
    if (!terminal->receive_text) return;

    // Apply font using CSS
    const char *font_desc_str = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(terminal->font_button));
    PangoFontDescription *font_desc = pango_font_description_from_string(font_desc_str ? font_desc_str : "Monospace 10");

    const char *family = pango_font_description_get_family(font_desc);
    int size = pango_font_description_get_size(font_desc) / PANGO_SCALE;

    // Apply colors using CSS
    GdkRGBA bg_color, text_color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(terminal->bg_color_button), &bg_color);
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(terminal->text_color_button), &text_color);

    // Create CSS string with more specific selectors
    char css_data[1024];
    snprintf(css_data, sizeof(css_data),
        "textview, textview text { "
        "font-family: \"%s\"; "
        "font-size: %dpt; "
        "background-color: rgba(%d,%d,%d,%.2f); "
        "color: rgba(%d,%d,%d,%.2f); "
        "} "
        "textview:selected { "
        "background-color: rgba(%d,%d,%d,0.3); "
        "}",
        family ? family : "Monospace",
        size > 0 ? size : 10,
        (int)(bg_color.red * 255), (int)(bg_color.green * 255), (int)(bg_color.blue * 255), bg_color.alpha,
        (int)(text_color.red * 255), (int)(text_color.green * 255), (int)(text_color.blue * 255), text_color.alpha,
        (int)(text_color.red * 255), (int)(text_color.green * 255), (int)(text_color.blue * 255)
    );

    pango_font_description_free(font_desc);

    // Apply CSS with higher priority
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error = NULL;
    gtk_css_provider_load_from_data(css_provider, css_data, -1, &error);

    if (error) {
        g_warning("CSS Error: %s", error->message);
        g_error_free(error);
    }

    // Apply CSS to text display
    GtkStyleContext *text_context = gtk_widget_get_style_context(terminal->receive_text);
    gtk_style_context_add_provider(text_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Apply CSS to hex display if it exists (even if hidden)
    if (terminal->hex_text) {
        GtkStyleContext *hex_context = gtk_widget_get_style_context(terminal->hex_text);
        gtk_style_context_add_provider(hex_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        // Force the hex text view to realize its style even when hidden
        gtk_widget_realize(terminal->hex_text);
    }

    g_object_unref(css_provider);

    // Apply theme
    const char *theme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo));
    if (theme) {
        GtkSettings *settings = gtk_settings_get_default();
        if (strcmp(theme, "Dark") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
            // Force theme update
            g_object_set(settings, "gtk-theme-name", "Adwaita-dark", NULL);
        } else if (strcmp(theme, "Light") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            // Force theme update
            g_object_set(settings, "gtk-theme-name", "Adwaita", NULL);
        } else {
            // System default - reset to system preference
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            g_object_set(settings, "gtk-theme-name", NULL, NULL);
        }

        // Force a style update on all widgets
        GList *toplevels = gtk_window_list_toplevels();
        for (GList *l = toplevels; l != NULL; l = l->next) {
            if (GTK_IS_WIDGET(l->data)) {
                gtk_widget_reset_style(GTK_WIDGET(l->data));
            }
        }
        g_list_free(toplevels);
    }
}

void apply_theme_setting(SerialTerminal *terminal) {
    const char *theme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo));
    if (theme) {
        GtkSettings *settings = gtk_settings_get_default();
        if (strcmp(theme, "Dark") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
            g_object_set(settings, "gtk-theme-name", "Adwaita-dark", NULL);
        } else if (strcmp(theme, "Light") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            g_object_set(settings, "gtk-theme-name", "Adwaita", NULL);
        } else {
            // System default
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            g_object_set(settings, "gtk-theme-name", NULL, NULL);
        }

        // Force immediate style update
        GList *toplevels = gtk_window_list_toplevels();
        for (GList *l = toplevels; l != NULL; l = l->next) {
            if (GTK_IS_WIDGET(l->data)) {
                gtk_widget_reset_style(GTK_WIDGET(l->data));
            }
        }
        g_list_free(toplevels);
    }
}

void create_menu_bar(SerialTerminal *terminal, GtkWidget *parent) {
    // Create menu bar
    terminal->menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(parent), terminal->menu_bar, FALSE, FALSE, 0);

    // File menu
    GtkWidget *file_menu_item = gtk_menu_item_new_with_label("File");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), file_menu_item);

    terminal->file_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), terminal->file_menu);

    GtkWidget *exit_item = gtk_menu_item_new_with_label("Exit");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->file_menu), exit_item);
    g_signal_connect(exit_item, "activate", G_CALLBACK(on_file_exit_activate), terminal);

    // View menu
    GtkWidget *view_menu_item = gtk_menu_item_new_with_label("View");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), view_menu_item);

    terminal->view_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_item), terminal->view_menu);

    GtkWidget *appearance_item = gtk_menu_item_new_with_label("Appearance...");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->view_menu), appearance_item);
    g_signal_connect(appearance_item, "activate", G_CALLBACK(on_view_appearance_activate), terminal);

    GtkWidget *display_options_item = gtk_menu_item_new_with_label("Display Options...");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->view_menu), display_options_item);
    g_signal_connect(display_options_item, "activate", G_CALLBACK(on_view_display_options_activate), terminal);

    // Tools menu
    GtkWidget *tools_menu_item = gtk_menu_item_new_with_label("Tools");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), tools_menu_item);

    terminal->tools_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools_menu_item), terminal->tools_menu);

    GtkWidget *bridge_item = gtk_menu_item_new_with_label("BRIDGE");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->tools_menu), bridge_item);
    g_signal_connect(bridge_item, "activate", G_CALLBACK(on_tools_bridge_activate), terminal);

    // Macros menu
    GtkWidget *macros_menu_item = gtk_menu_item_new_with_label("Macros");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), macros_menu_item);

    terminal->macros_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(macros_menu_item), terminal->macros_menu);

    GtkWidget *program_macros_item = gtk_menu_item_new_with_label("Program Macros...");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->macros_menu), program_macros_item);
    g_signal_connect(program_macros_item, "activate", G_CALLBACK(on_macros_program_activate), terminal);

    // Add separator
    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->macros_menu), separator);

    // Add show/hide toggle
    GtkWidget *toggle_macros_item = gtk_menu_item_new_with_label("Show/Hide Macro Panel");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->macros_menu), toggle_macros_item);
    g_signal_connect(toggle_macros_item, "activate", G_CALLBACK(on_macros_toggle_activate), terminal);

    // Help menu
    GtkWidget *help_menu_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), help_menu_item);

    terminal->help_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), terminal->help_menu);

    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->help_menu), about_item);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_help_about_activate), terminal);
}

void populate_interval_dropdown_for_repeat(SerialTerminal *terminal) {
    // Clear existing items
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo));

    // Add repeat mode intervals (seconds)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.2");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "1.0");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "2.0");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "5.0");

    // Set default to 1.0 second
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->send_file_interval_combo), 3);
}

void populate_interval_dropdown_for_lines(SerialTerminal *terminal) {
    // Clear existing items
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo));

    // Add line-by-line mode intervals (milliseconds)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "10");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "25");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "50");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "100");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "200");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "250");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "500");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "1000");

    // Set default to 100ms
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->send_file_interval_combo), 4);
}

void update_interval_dropdown_based_on_mode(SerialTerminal *terminal) {
    gboolean lines_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->send_file_lines_check));

    if (lines_mode) {
        populate_interval_dropdown_for_lines(terminal);
        gtk_label_set_text(GTK_LABEL(terminal->send_file_interval_label), "ms");
    } else {
        populate_interval_dropdown_for_repeat(terminal);
        gtk_label_set_text(GTK_LABEL(terminal->send_file_interval_label), "sec");
    }
}

void create_macro_panel(SerialTerminal *terminal, GtkWidget *parent) {
    // Create frame for macros
    GtkWidget *frame = gtk_frame_new("Macros");
    gtk_box_pack_start(GTK_BOX(parent), frame, TRUE, TRUE, 5);

    // Create scrolled window for macro buttons
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(frame), scrolled);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled), 5);

    // Create vertical box for macro buttons
    GtkWidget *macro_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_add(GTK_CONTAINER(scrolled), macro_vbox);

    // Create macro buttons
    for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
        char default_label[32];
        snprintf(default_label, sizeof(default_label), "Macro %d", i + 1);

        terminal->macro_buttons[i] = gtk_button_new_with_label(default_label);
        gtk_box_pack_start(GTK_BOX(macro_vbox), terminal->macro_buttons[i], FALSE, FALSE, 2);

        // Set button size
        gtk_widget_set_size_request(terminal->macro_buttons[i], 180, 30);

        // Store button index as data for callback
        g_object_set_data(G_OBJECT(terminal->macro_buttons[i]), "macro_index", GINT_TO_POINTER(i));

        // Connect signal
        g_signal_connect(terminal->macro_buttons[i], "clicked", G_CALLBACK(on_macro_button_clicked), terminal);

        // Initialize macro data
        strncpy(terminal->macro_labels[i], default_label, MAX_MACRO_LABEL_LENGTH - 1);
        terminal->macro_labels[i][MAX_MACRO_LABEL_LENGTH - 1] = '\0';
        terminal->macro_commands[i][0] = '\0'; // Empty command initially
    }
}

void show_macro_programming_dialog(SerialTerminal *terminal) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Program Macros",
                                                   GTK_WINDOW(terminal->window),
                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_OK", GTK_RESPONSE_OK,
                                                   NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 600);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);

    // Create scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(content_area), scrolled, TRUE, TRUE, 0);

    // Create grid for macro entries
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(scrolled), grid);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    // Create entry widgets for each macro
    GtkWidget *label_entries[MAX_MACRO_BUTTONS];
    GtkWidget *command_entries[MAX_MACRO_BUTTONS];

    // Add headers
    GtkWidget *header_macro = gtk_label_new("Macro");
    GtkWidget *header_label = gtk_label_new("Button Label");
    GtkWidget *header_command = gtk_label_new("Command to Send");

    gtk_widget_set_halign(header_macro, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(header_label, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(header_command, GTK_ALIGN_CENTER);

    // Make headers bold
    PangoAttrList *attrs = pango_attr_list_new();
    PangoAttribute *bold = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attrs, bold);
    gtk_label_set_attributes(GTK_LABEL(header_macro), attrs);
    gtk_label_set_attributes(GTK_LABEL(header_label), pango_attr_list_copy(attrs));
    gtk_label_set_attributes(GTK_LABEL(header_command), pango_attr_list_copy(attrs));
    pango_attr_list_unref(attrs);

    gtk_grid_attach(GTK_GRID(grid), header_macro, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_command, 2, 0, 1, 1);

    for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
        // Macro number label
        char macro_num[16];
        snprintf(macro_num, sizeof(macro_num), "%d", i + 1);
        GtkWidget *num_label = gtk_label_new(macro_num);
        gtk_widget_set_halign(num_label, GTK_ALIGN_CENTER);
        gtk_grid_attach(GTK_GRID(grid), num_label, 0, i + 1, 1, 1);

        // Label entry
        label_entries[i] = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(label_entries[i]), terminal->macro_labels[i]);
        gtk_entry_set_max_length(GTK_ENTRY(label_entries[i]), MAX_MACRO_LABEL_LENGTH - 1);
        gtk_widget_set_size_request(label_entries[i], 120, -1);
        gtk_grid_attach(GTK_GRID(grid), label_entries[i], 1, i + 1, 1, 1);

        // Command entry
        command_entries[i] = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(command_entries[i]), terminal->macro_commands[i]);
        gtk_entry_set_max_length(GTK_ENTRY(command_entries[i]), MAX_MACRO_COMMAND_LENGTH - 1);
        gtk_widget_set_size_request(command_entries[i], 250, -1);
        gtk_grid_attach(GTK_GRID(grid), command_entries[i], 2, i + 1, 1, 1);
    }

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        // Save the macro settings
        for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
            const char *label = gtk_entry_get_text(GTK_ENTRY(label_entries[i]));
            const char *command = gtk_entry_get_text(GTK_ENTRY(command_entries[i]));

            strncpy(terminal->macro_labels[i], label, MAX_MACRO_LABEL_LENGTH - 1);
            terminal->macro_labels[i][MAX_MACRO_LABEL_LENGTH - 1] = '\0';

            strncpy(terminal->macro_commands[i], command, MAX_MACRO_COMMAND_LENGTH - 1);
            terminal->macro_commands[i][MAX_MACRO_COMMAND_LENGTH - 1] = '\0';

            // Update button label
            gtk_button_set_label(GTK_BUTTON(terminal->macro_buttons[i]), terminal->macro_labels[i]);
        }

        // Save settings to file
        save_settings(terminal);
    }

    gtk_widget_destroy(dialog);
}

void toggle_macro_panel_visibility(SerialTerminal *terminal) {
    if (terminal->macro_panel_visible) {
        // Hide the macro panel
        gtk_widget_hide(terminal->macro_panel);
        terminal->macro_panel_visible = FALSE;
    } else {
        // Show the macro panel
        gtk_widget_show(terminal->macro_panel);
        terminal->macro_panel_visible = TRUE;
    }

    // Save the visibility state
    save_settings(terminal);
}
