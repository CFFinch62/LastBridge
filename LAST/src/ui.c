/*
 * User Interface module for LAST - Linux Advanced Serial Transceiver
 * Handles GTK UI creation and layout
 */

#include "ui.h"
#include "callbacks.h"

void create_main_interface(SerialTerminal *terminal) {
    // Create main window - reduce size by 15-20%
    terminal->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(terminal->window), "LAST - Linux Advanced Serial Transceiver");
    gtk_window_set_default_size(GTK_WINDOW(terminal->window), 1190, 600); // Adjusted for 20px borders on both sides

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

    // Create main horizontal layout with three columns
    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), main_hbox, TRUE, TRUE, 5);

    // Create left panel (Appearance and File Operations)
    GtkWidget *left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(left_panel, 300, -1);
    gtk_box_pack_start(GTK_BOX(main_hbox), left_panel, FALSE, FALSE, 20); // Add 20px left margin

    // Create appearance panel
    create_appearance_panel(terminal, left_panel);

    // Create file operations panel
    create_file_operations_panel(terminal, left_panel);

    // Create center panel (Data area and send controls) - optimized width
    GtkWidget *center_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(center_panel, 480, -1); // Adjusted for better fit in smaller window
    gtk_box_pack_start(GTK_BOX(main_hbox), center_panel, FALSE, FALSE, 0);

    // Create data area in center
    create_data_area(terminal, center_panel);

    // Create right panel (Connection, Display Options, Control Signals) - fixed width
    GtkWidget *right_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(right_panel, 320, -1); // Slightly wider for better content fit
    gtk_box_pack_start(GTK_BOX(main_hbox), right_panel, FALSE, FALSE, 20); // Add 20px right margin

    // Create connection panel
    create_connection_panel(terminal, right_panel);

    // Create display options panel
    create_display_options_panel(terminal, right_panel);

    // Create control signals panel
    create_control_signals_panel(terminal, right_panel);

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

    // Port selection
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Port:"), 0, row, 1, 1);
    terminal->port_combo = gtk_combo_box_text_new();
    gtk_grid_attach(GTK_GRID(grid), terminal->port_combo, 1, row, 1, 1);

    terminal->refresh_button = gtk_button_new_with_label("Refresh");
    gtk_grid_attach(GTK_GRID(grid), terminal->refresh_button, 2, row, 1, 1);
    row++;

    // Baud rate
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Baud Rate:"), 0, row, 1, 1);
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
    gtk_grid_attach(GTK_GRID(grid), terminal->baudrate_combo, 1, row, 2, 1);
    row++;

    // Data bits
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Data Bits:"), 0, row, 1, 1);
    terminal->databits_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "6");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "7");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo), "8");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->databits_combo), 3); // 8
    gtk_grid_attach(GTK_GRID(grid), terminal->databits_combo, 1, row, 2, 1);
    row++;

    // Parity
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Parity:"), 0, row, 1, 1);
    terminal->parity_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "Even");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo), "Odd");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->parity_combo), 0); // None
    gtk_grid_attach(GTK_GRID(grid), terminal->parity_combo, 1, row, 2, 1);
    row++;

    // Stop bits
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Stop Bits:"), 0, row, 1, 1);
    terminal->stopbits_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo), "1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo), "2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->stopbits_combo), 0); // 1
    gtk_grid_attach(GTK_GRID(grid), terminal->stopbits_combo, 1, row, 2, 1);
    row++;

    // Flow control
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Flow Control:"), 0, row, 1, 1);
    terminal->flowcontrol_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "Hardware");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo), "Software");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->flowcontrol_combo), 0); // None
    gtk_grid_attach(GTK_GRID(grid), terminal->flowcontrol_combo, 1, row, 2, 1);
    row++;

    // Connection buttons
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

    // Repeat controls
    terminal->send_file_repeat_check = gtk_check_button_new_with_label("Repeat sending");
    gtk_box_pack_start(GTK_BOX(send_file_vbox), terminal->send_file_repeat_check, FALSE, FALSE, 0);

    GtkWidget *interval_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(send_file_vbox), interval_hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(interval_hbox), gtk_label_new("Interval:"), FALSE, FALSE, 0);

    terminal->send_file_interval_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.2");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "1.0");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "2.0");
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->send_file_interval_combo), 3); // 1.0 second default
    gtk_box_pack_start(GTK_BOX(interval_hbox), terminal->send_file_interval_combo, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(interval_hbox), gtk_label_new("sec"), FALSE, FALSE, 0);

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
    // Set a reasonable height for the receive area
    gtk_widget_set_size_request(scrolled, -1, 300);
    gtk_box_pack_start(GTK_BOX(receive_vbox), scrolled, TRUE, TRUE, 0);

    terminal->receive_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(terminal->receive_text), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(terminal->receive_text), TRUE);
    // Disable text wrapping to enable horizontal scrolling
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(terminal->receive_text), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(scrolled), terminal->receive_text);

    // Receive controls
    GtkWidget *receive_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(receive_vbox), receive_controls, FALSE, FALSE, 0);

    terminal->clear_button = gtk_button_new_with_label("Clear");
    gtk_box_pack_start(GTK_BOX(receive_controls), terminal->clear_button, FALSE, FALSE, 0);

    terminal->save_button = gtk_button_new_with_label("Save Received Data...");
    gtk_box_pack_start(GTK_BOX(receive_controls), terminal->save_button, FALSE, FALSE, 0);

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

    GtkStyleContext *context = gtk_widget_get_style_context(terminal->receive_text);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

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

    // Tools menu
    GtkWidget *tools_menu_item = gtk_menu_item_new_with_label("Tools");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), tools_menu_item);

    terminal->tools_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools_menu_item), terminal->tools_menu);

    GtkWidget *bridge_item = gtk_menu_item_new_with_label("BRIDGE");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->tools_menu), bridge_item);
    g_signal_connect(bridge_item, "activate", G_CALLBACK(on_tools_bridge_activate), terminal);

    // Help menu
    GtkWidget *help_menu_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), help_menu_item);

    terminal->help_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), terminal->help_menu);

    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->help_menu), about_item);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_help_about_activate), terminal);
}
