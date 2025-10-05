/*
 * User interface module for BRIDGE - Virtual Null Modem Bridge
 * GTK3 interface creation and management
 */

#include "ui.h"
#include "utils.h"

void create_main_window(BridgeApp *app) {
    // Create main window
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "BRIDGE - Virtual Null Modem");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 600, 500);
    gtk_window_set_position(GTK_WINDOW(app->window), GTK_WIN_POS_CENTER);

    // Set window icon - try multiple approaches
    GdkPixbuf *icon = NULL;
    GError *error = NULL;

    // Try loading PNG icon first (smaller, better for window icons)
    const char *icon_paths[] = {
        "bridge-icon.png",                           // Current directory (PNG)
        "bridge-icon.jpg",                           // Current directory (JPG fallback)
        "/usr/local/share/pixmaps/bridge-icon.png",  // System location (PNG)
        "/usr/local/share/pixmaps/bridge-icon.jpg",  // System location (JPG fallback)
        "/usr/share/pixmaps/bridge-icon.png",        // Alternative system location (PNG)
        "/usr/share/pixmaps/bridge-icon.jpg",        // Alternative system location (JPG fallback)
        NULL
    };

    for (int i = 0; icon_paths[i] != NULL && !icon; i++) {
        error = NULL;
        icon = gdk_pixbuf_new_from_file(icon_paths[i], &error);
        if (icon) {
            // Scale icon to appropriate size for window decoration
            GdkPixbuf *scaled_icon = gdk_pixbuf_scale_simple(icon, 48, 48, GDK_INTERP_BILINEAR);
            if (scaled_icon) {
                gtk_window_set_icon(GTK_WINDOW(app->window), scaled_icon);
                g_object_unref(scaled_icon);
            }
            g_object_unref(icon);
            break;
        } else if (error) {
            g_error_free(error);
        }
    }

    // Create main vertical box
    app->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app->window), app->main_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(app->main_vbox), 10);

    // Create notebook for tabs
    app->notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(app->main_vbox), app->notebook, TRUE, TRUE, 0);

    // Create tabs
    create_configuration_tab(app, app->notebook);
    create_status_tab(app, app->notebook);
    create_sniffing_tab(app, app->notebook);
    create_settings_tab(app, app->notebook);

    // Apply theme
    apply_ui_theme(app);
}

void create_configuration_tab(BridgeApp *app, GtkWidget *notebook) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>Virtual Null Modem Configuration</b>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    // Device configuration frame
    GtkWidget *config_frame = gtk_frame_new("Device Configuration");
    gtk_box_pack_start(GTK_BOX(vbox), config_frame, FALSE, FALSE, 0);

    GtkWidget *config_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(config_frame), config_grid);
    gtk_container_set_border_width(GTK_CONTAINER(config_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(config_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(config_grid), 10);

    // Device 1
    GtkWidget *device1_label = gtk_label_new("Device 1:");
    gtk_grid_attach(GTK_GRID(config_grid), device1_label, 0, 0, 1, 1);
    
    app->device1_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->device1_entry), DEFAULT_DEVICE1);
    gtk_grid_attach(GTK_GRID(config_grid), app->device1_entry, 1, 0, 1, 1);

    // Device 2
    GtkWidget *device2_label = gtk_label_new("Device 2:");
    gtk_grid_attach(GTK_GRID(config_grid), device2_label, 0, 1, 1, 1);
    
    app->device2_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->device2_entry), DEFAULT_DEVICE2);
    gtk_grid_attach(GTK_GRID(config_grid), app->device2_entry, 1, 1, 1, 1);

    // Control buttons frame
    GtkWidget *control_frame = gtk_frame_new("Control");
    gtk_box_pack_start(GTK_BOX(vbox), control_frame, FALSE, FALSE, 0);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(control_frame), button_box);
    gtk_container_set_border_width(GTK_CONTAINER(button_box), 10);

    app->start_button = gtk_button_new_with_label("Start Virtual Null Modem");
    gtk_box_pack_start(GTK_BOX(button_box), app->start_button, FALSE, FALSE, 0);

    app->stop_button = gtk_button_new_with_label("Stop Virtual Null Modem");
    gtk_box_pack_start(GTK_BOX(button_box), app->stop_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(app->stop_button, FALSE);

    app->test_button = gtk_button_new_with_label("Test Communication");
    gtk_box_pack_start(GTK_BOX(button_box), app->test_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(app->test_button, FALSE);

    // Instructions frame
    GtkWidget *instructions_frame = gtk_frame_new("Usage Instructions");
    gtk_box_pack_start(GTK_BOX(vbox), instructions_frame, FALSE, FALSE, 0);

    GtkWidget *instructions_label = gtk_label_new(
        "1. Configure device paths above (default: /tmp/ttyV0 and /tmp/ttyV1)\n"
        "2. Click 'Start Virtual Null Modem' to create the devices\n"
        "3. Use the device paths in your applications (like LAST)\n"
        "4. Data sent to one device will appear on the other\n"
        "5. Click 'Test Communication' to verify the connection works");
    gtk_label_set_justify(GTK_LABEL(instructions_label), GTK_JUSTIFY_LEFT);
    gtk_container_add(GTK_CONTAINER(instructions_frame), instructions_label);
    gtk_container_set_border_width(GTK_CONTAINER(instructions_frame), 10);

    // Add tab to notebook
    GtkWidget *tab_label = gtk_label_new("Configuration");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);
}

void create_status_tab(BridgeApp *app, GtkWidget *notebook) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

    // Status frame
    GtkWidget *status_frame = gtk_frame_new("Status");
    gtk_box_pack_start(GTK_BOX(vbox), status_frame, FALSE, FALSE, 0);

    GtkWidget *status_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(status_frame), status_grid);
    gtk_container_set_border_width(GTK_CONTAINER(status_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(status_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(status_grid), 10);

    // Status
    GtkWidget *status_text_label = gtk_label_new("Status:");
    gtk_grid_attach(GTK_GRID(status_grid), status_text_label, 0, 0, 1, 1);
    
    app->status_label = gtk_label_new("Stopped");
    gtk_grid_attach(GTK_GRID(status_grid), app->status_label, 1, 0, 1, 1);

    // Active devices
    GtkWidget *devices_text_label = gtk_label_new("Active Devices:");
    gtk_grid_attach(GTK_GRID(status_grid), devices_text_label, 0, 1, 1, 1);
    
    app->devices_label = gtk_label_new("None");
    gtk_grid_attach(GTK_GRID(status_grid), app->devices_label, 1, 1, 1, 1);

    // Connection time
    GtkWidget *time_text_label = gtk_label_new("Uptime:");
    gtk_grid_attach(GTK_GRID(status_grid), time_text_label, 0, 2, 1, 1);
    
    app->connection_time_label = gtk_label_new("00:00:00");
    gtk_grid_attach(GTK_GRID(status_grid), app->connection_time_label, 1, 2, 1, 1);

    // Log frame
    GtkWidget *log_frame = gtk_frame_new("Log");
    gtk_box_pack_start(GTK_BOX(vbox), log_frame, TRUE, TRUE, 0);

    GtkWidget *log_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(log_frame), log_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(log_vbox), 10);

    // Log text view
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(log_vbox), scrolled, TRUE, TRUE, 0);

    app->log_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->log_text), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(app->log_text), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled), app->log_text);

    app->log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->log_text));

    // Clear log button
    app->clear_log_button = gtk_button_new_with_label("Clear Log");
    gtk_box_pack_start(GTK_BOX(log_vbox), app->clear_log_button, FALSE, FALSE, 0);

    // Add tab to notebook
    GtkWidget *tab_label = gtk_label_new("Status & Log");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);
}

void create_settings_tab(BridgeApp *app, GtkWidget *notebook) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

    // Settings frame
    GtkWidget *settings_frame = gtk_frame_new("Application Settings");
    gtk_box_pack_start(GTK_BOX(vbox), settings_frame, FALSE, FALSE, 0);

    GtkWidget *settings_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(settings_frame), settings_grid);
    gtk_container_set_border_width(GTK_CONTAINER(settings_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(settings_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(settings_grid), 10);

    // Auto-start checkbox
    app->auto_start_check = gtk_check_button_new_with_label("Auto-start null modem on application start");
    gtk_grid_attach(GTK_GRID(settings_grid), app->auto_start_check, 0, 0, 2, 1);

    // Verbose logging checkbox
    app->verbose_logging_check = gtk_check_button_new_with_label("Enable verbose console logging");
    gtk_grid_attach(GTK_GRID(settings_grid), app->verbose_logging_check, 0, 1, 2, 1);

    // Device permissions
    GtkWidget *permissions_label = gtk_label_new("Device Permissions:");
    gtk_grid_attach(GTK_GRID(settings_grid), permissions_label, 0, 2, 1, 1);

    app->device_permissions_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->device_permissions_combo), "Default");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->device_permissions_combo), "666 (rw-rw-rw-)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->device_permissions_combo), "644 (rw-r--r--)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->device_permissions_combo), "600 (rw-------)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->device_permissions_combo), 0);
    gtk_grid_attach(GTK_GRID(settings_grid), app->device_permissions_combo, 1, 2, 1, 1);

    // Add tab to notebook
    GtkWidget *tab_label = gtk_label_new("Settings");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);
}

void create_sniffing_tab(BridgeApp *app, GtkWidget *notebook) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>Serial Data Sniffing</b>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    // Enable sniffing frame
    GtkWidget *enable_frame = gtk_frame_new("Sniffing Control");
    gtk_box_pack_start(GTK_BOX(vbox), enable_frame, FALSE, FALSE, 0);

    GtkWidget *enable_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(enable_frame), enable_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(enable_vbox), 10);

    app->sniffing_enable_check = gtk_check_button_new_with_label("Enable Serial Data Sniffing");
    gtk_box_pack_start(GTK_BOX(enable_vbox), app->sniffing_enable_check, FALSE, FALSE, 0);

    // Output methods frame
    GtkWidget *output_frame = gtk_frame_new("Output Methods");
    gtk_box_pack_start(GTK_BOX(vbox), output_frame, FALSE, FALSE, 0);

    GtkWidget *output_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(output_frame), output_grid);
    gtk_container_set_border_width(GTK_CONTAINER(output_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(output_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(output_grid), 10);

    // Named Pipe
    app->sniff_pipe_check = gtk_check_button_new_with_label("Named Pipe:");
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_pipe_check, 0, 0, 1, 1);

    app->sniff_pipe_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->sniff_pipe_entry), DEFAULT_SNIFF_PIPE);
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_pipe_entry, 1, 0, 1, 1);

    // TCP Socket
    app->sniff_tcp_check = gtk_check_button_new_with_label("TCP Socket Port:");
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_tcp_check, 0, 1, 1, 1);

    app->sniff_tcp_port_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->sniff_tcp_port_entry), "8888");
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_tcp_port_entry, 1, 1, 1, 1);

    // UDP Socket
    app->sniff_udp_check = gtk_check_button_new_with_label("UDP Address:");
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_udp_check, 0, 2, 1, 1);

    app->sniff_udp_addr_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->sniff_udp_addr_entry), DEFAULT_SNIFF_UDP_ADDR);
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_udp_addr_entry, 1, 2, 1, 1);

    GtkWidget *udp_port_label = gtk_label_new("UDP Port:");
    gtk_grid_attach(GTK_GRID(output_grid), udp_port_label, 0, 3, 1, 1);

    app->sniff_udp_port_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->sniff_udp_port_entry), "9999");
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_udp_port_entry, 1, 3, 1, 1);

    // Log File
    app->sniff_file_check = gtk_check_button_new_with_label("Log File:");
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_file_check, 0, 4, 1, 1);

    app->sniff_file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->sniff_file_entry), "Auto-generated filename");
    gtk_grid_attach(GTK_GRID(output_grid), app->sniff_file_entry, 1, 4, 1, 1);

    // Configuration frame
    GtkWidget *config_frame = gtk_frame_new("Sniffing Configuration");
    gtk_box_pack_start(GTK_BOX(vbox), config_frame, FALSE, FALSE, 0);

    GtkWidget *config_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(config_frame), config_grid);
    gtk_container_set_border_width(GTK_CONTAINER(config_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(config_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(config_grid), 10);

    // Data Direction
    GtkWidget *direction_label = gtk_label_new("Data Direction:");
    gtk_grid_attach(GTK_GRID(config_grid), direction_label, 0, 0, 1, 1);

    app->sniff_direction_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->sniff_direction_combo), "Both RX & TX");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->sniff_direction_combo), "RX Only");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->sniff_direction_combo), "TX Only");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->sniff_direction_combo), 0);
    gtk_grid_attach(GTK_GRID(config_grid), app->sniff_direction_combo, 1, 0, 1, 1);

    // Data Format
    GtkWidget *format_label = gtk_label_new("Data Format:");
    gtk_grid_attach(GTK_GRID(config_grid), format_label, 0, 1, 1, 1);

    app->sniff_format_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->sniff_format_combo), "Raw Binary");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->sniff_format_combo), "Hex Dump");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->sniff_format_combo), "Text");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->sniff_format_combo), 1); // Default to Hex
    gtk_grid_attach(GTK_GRID(config_grid), app->sniff_format_combo, 1, 1, 1, 1);

    // Control buttons frame
    GtkWidget *control_frame = gtk_frame_new("Sniffing Control");
    gtk_box_pack_start(GTK_BOX(vbox), control_frame, FALSE, FALSE, 0);

    GtkWidget *control_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(control_frame), control_hbox);
    gtk_container_set_border_width(GTK_CONTAINER(control_hbox), 10);

    app->sniff_start_button = gtk_button_new_with_label("Start Sniffing");
    gtk_box_pack_start(GTK_BOX(control_hbox), app->sniff_start_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(app->sniff_start_button, FALSE);

    app->sniff_stop_button = gtk_button_new_with_label("Stop Sniffing");
    gtk_box_pack_start(GTK_BOX(control_hbox), app->sniff_stop_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(app->sniff_stop_button, FALSE);

    // Statistics frame
    GtkWidget *stats_frame = gtk_frame_new("Sniffing Statistics");
    gtk_box_pack_start(GTK_BOX(vbox), stats_frame, FALSE, FALSE, 0);

    app->sniff_stats_label = gtk_label_new("Sniffing inactive");
    gtk_container_add(GTK_CONTAINER(stats_frame), app->sniff_stats_label);
    gtk_container_set_border_width(GTK_CONTAINER(stats_frame), 10);

    // Instructions frame
    GtkWidget *sniff_instructions_frame = gtk_frame_new("Usage Instructions");
    gtk_box_pack_start(GTK_BOX(vbox), sniff_instructions_frame, FALSE, FALSE, 0);

    GtkWidget *sniff_instructions_label = gtk_label_new(
        "1. Enable sniffing and select desired output methods\n"
        "2. Configure output paths/ports as needed\n"
        "3. Start the null modem bridge first\n"
        "4. Click 'Start Sniffing' to begin data capture\n"
        "5. Connect your applications to the bridge devices\n"
        "6. All serial data will be streamed to selected outputs");
    gtk_label_set_justify(GTK_LABEL(sniff_instructions_label), GTK_JUSTIFY_LEFT);
    gtk_container_add(GTK_CONTAINER(sniff_instructions_frame), sniff_instructions_label);
    gtk_container_set_border_width(GTK_CONTAINER(sniff_instructions_frame), 10);

    // Add tab to notebook
    GtkWidget *tab_label = gtk_label_new("Sniffing");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);
}

gboolean update_ui_state(gpointer data) {
    BridgeApp *app = (BridgeApp *)data;
    const char *status_text = "Unknown";
    const char *status_color = "black";

    switch (app->state) {
        case BRIDGE_STATE_STOPPED:
            status_text = "Stopped";
            status_color = "red";
            gtk_widget_set_sensitive(app->start_button, TRUE);
            gtk_widget_set_sensitive(app->stop_button, FALSE);
            gtk_widget_set_sensitive(app->test_button, FALSE);
            gtk_widget_set_sensitive(app->sniff_start_button, FALSE);
            gtk_label_set_text(GTK_LABEL(app->devices_label), "None");
            break;

        case BRIDGE_STATE_STARTING:
            status_text = "Starting...";
            status_color = "orange";
            gtk_widget_set_sensitive(app->start_button, FALSE);
            gtk_widget_set_sensitive(app->stop_button, FALSE);
            gtk_widget_set_sensitive(app->test_button, FALSE);
            break;

        case BRIDGE_STATE_RUNNING:
            status_text = "Running";
            status_color = "green";
            gtk_widget_set_sensitive(app->start_button, FALSE);
            gtk_widget_set_sensitive(app->stop_button, TRUE);
            gtk_widget_set_sensitive(app->test_button, TRUE);
            gtk_widget_set_sensitive(app->sniff_start_button,
                                   app->sniffing_enabled && !is_sniffing_active(app));

            char devices_text[512];
            snprintf(devices_text, sizeof(devices_text), "%.240s ↔ %.240s",
                    app->device1_path, app->device2_path);
            gtk_label_set_text(GTK_LABEL(app->devices_label), devices_text);
            break;

        case BRIDGE_STATE_STOPPING:
            status_text = "Stopping...";
            status_color = "orange";
            gtk_widget_set_sensitive(app->start_button, FALSE);
            gtk_widget_set_sensitive(app->stop_button, FALSE);
            gtk_widget_set_sensitive(app->test_button, FALSE);
            break;

        case BRIDGE_STATE_ERROR:
            status_text = "Error";
            status_color = "red";
            gtk_widget_set_sensitive(app->start_button, TRUE);
            gtk_widget_set_sensitive(app->stop_button, FALSE);
            gtk_widget_set_sensitive(app->test_button, FALSE);
            gtk_label_set_text(GTK_LABEL(app->devices_label), "None");
            break;
    }

    // Update status label with color
    char markup[256];
    snprintf(markup, sizeof(markup), "<span color=\"%s\">%s</span>", status_color, status_text);
    gtk_label_set_markup(GTK_LABEL(app->status_label), markup);

    return FALSE; // Don't repeat if called from g_idle_add
}

void append_log_message(BridgeApp *app, const char *message, gboolean timestamp) {
    if (!app->log_buffer) return;
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->log_buffer, &iter);
    
    if (timestamp) {
        char *ts = get_current_timestamp();
        char full_message[MAX_LOG_LENGTH + 64];
        snprintf(full_message, sizeof(full_message), "[%s] %s\n", ts, message);
        gtk_text_buffer_insert(app->log_buffer, &iter, full_message, -1);
        free(ts);
    } else {
        gtk_text_buffer_insert(app->log_buffer, &iter, message, -1);
        gtk_text_buffer_insert(app->log_buffer, &iter, "\n", -1);
    }
    
    // Auto-scroll to bottom
    GtkTextMark *mark = gtk_text_buffer_get_insert(app->log_buffer);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(app->log_text), mark);
    
    // Limit log length
    gint line_count = gtk_text_buffer_get_line_count(app->log_buffer);
    if (line_count > 500) {
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(app->log_buffer, &start);
        gtk_text_buffer_get_iter_at_line(app->log_buffer, &end, 100);
        gtk_text_buffer_delete(app->log_buffer, &start, &end);
    }
}

void clear_log(BridgeApp *app) {
    if (app->log_buffer) {
        gtk_text_buffer_set_text(app->log_buffer, "", -1);
    }
}

void apply_ui_theme(BridgeApp *app) {
    // Apply theme settings if available
    // This would be expanded based on the appearance settings
    // For now, just ensure proper styling
    (void)app; // Suppress unused parameter warning
}
