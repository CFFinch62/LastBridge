/*
 * Callbacks module for BRIDGE - Virtual Null Modem Bridge
 * GTK event handlers and signal connections
 */

#include "callbacks.h"
#include "nullmodem.h"
#include "sniffing.h"
#include "ui.h"
#include "settings.h"
#include "utils.h"

void connect_signals(BridgeApp *app) {
    // Window signals
    g_signal_connect(app->window, "destroy", G_CALLBACK(on_window_destroy), app);
    
    // Button signals
    g_signal_connect(app->start_button, "clicked", G_CALLBACK(on_start_button_clicked), app);
    g_signal_connect(app->stop_button, "clicked", G_CALLBACK(on_stop_button_clicked), app);
    g_signal_connect(app->test_button, "clicked", G_CALLBACK(on_test_button_clicked), app);
    g_signal_connect(app->clear_log_button, "clicked", G_CALLBACK(on_clear_log_clicked), app);
    
    // Entry signals
    g_signal_connect(app->device1_entry, "changed", G_CALLBACK(on_device_entry_changed), app);
    g_signal_connect(app->device2_entry, "changed", G_CALLBACK(on_device_entry_changed), app);
    
    // Settings signals
    g_signal_connect(app->auto_start_check, "toggled", G_CALLBACK(on_settings_changed), app);
    g_signal_connect(app->verbose_logging_check, "toggled", G_CALLBACK(on_settings_changed), app);
    g_signal_connect(app->device_permissions_combo, "changed", G_CALLBACK(on_settings_changed), app);

    // Sniffing signals
    g_signal_connect(app->sniffing_enable_check, "toggled", G_CALLBACK(on_sniffing_enable_toggled), app);
    g_signal_connect(app->sniff_start_button, "clicked", G_CALLBACK(on_sniff_start_clicked), app);
    g_signal_connect(app->sniff_stop_button, "clicked", G_CALLBACK(on_sniff_stop_clicked), app);
    g_signal_connect(app->sniff_pipe_check, "toggled", G_CALLBACK(on_sniff_output_toggled), app);
    g_signal_connect(app->sniff_tcp_check, "toggled", G_CALLBACK(on_sniff_output_toggled), app);
    g_signal_connect(app->sniff_udp_check, "toggled", G_CALLBACK(on_sniff_output_toggled), app);
    g_signal_connect(app->sniff_file_check, "toggled", G_CALLBACK(on_sniff_output_toggled), app);
    g_signal_connect(app->sniff_direction_combo, "changed", G_CALLBACK(on_sniff_settings_changed), app);
    g_signal_connect(app->sniff_format_combo, "changed", G_CALLBACK(on_sniff_settings_changed), app);
}

void on_start_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; // Suppress unused parameter warning
    BridgeApp *app = (BridgeApp *)user_data;
    
    // Update device paths from UI
    const char *device1 = gtk_entry_get_text(GTK_ENTRY(app->device1_entry));
    const char *device2 = gtk_entry_get_text(GTK_ENTRY(app->device2_entry));
    
    if (!device1 || !device2 || strlen(device1) == 0 || strlen(device2) == 0) {
        log_message(app, "ERROR: Please enter valid device paths");
        return;
    }
    
    if (strcmp(device1, device2) == 0) {
        log_message(app, "ERROR: Device paths must be different");
        return;
    }
    
    strncpy(app->device1_path, device1, MAX_PATH_LENGTH - 1);
    strncpy(app->device2_path, device2, MAX_PATH_LENGTH - 1);
    
    // Start null modem in a separate thread to avoid blocking UI
    GThread *start_thread = g_thread_new("start_nullmodem", 
                                        (GThreadFunc)start_nullmodem_thread, app);
    g_thread_unref(start_thread);
}

void on_stop_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; // Suppress unused parameter warning
    BridgeApp *app = (BridgeApp *)user_data;
    
    // Stop null modem in a separate thread
    GThread *stop_thread = g_thread_new("stop_nullmodem", 
                                       (GThreadFunc)stop_nullmodem_thread, app);
    g_thread_unref(stop_thread);
}

void on_test_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; // Suppress unused parameter warning
    BridgeApp *app = (BridgeApp *)user_data;
    
    // Test communication in a separate thread
    GThread *test_thread = g_thread_new("test_communication", 
                                       (GThreadFunc)test_communication_thread, app);
    g_thread_unref(test_thread);
}

void on_clear_log_clicked(GtkButton *button, gpointer user_data) {
    (void)button; // Suppress unused parameter warning
    BridgeApp *app = (BridgeApp *)user_data;
    
    clear_log(app);
    log_message(app, "Log cleared");
}

void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    (void)widget; // Suppress unused parameter warning
    BridgeApp *app = (BridgeApp *)user_data;
    
    // Save settings before exit
    save_settings(app);
    
    // Stop null modem if running
    if (app->state == BRIDGE_STATE_RUNNING) {
        stop_null_modem(app);
    }
    
    // Stop status timer
    if (app->status_timer_id > 0) {
        g_source_remove(app->status_timer_id);
    }
    
    gtk_main_quit();
}

void on_device_entry_changed(GtkEntry *entry, gpointer user_data) {
    (void)entry; // Suppress unused parameter warning
    BridgeApp *app = (BridgeApp *)user_data;
    
    // Update device paths when entries change
    const char *device1 = gtk_entry_get_text(GTK_ENTRY(app->device1_entry));
    const char *device2 = gtk_entry_get_text(GTK_ENTRY(app->device2_entry));
    
    strncpy(app->device1_path, device1, MAX_PATH_LENGTH - 1);
    strncpy(app->device2_path, device2, MAX_PATH_LENGTH - 1);
    
    // Save settings
    save_settings(app);
}

void on_settings_changed(GtkWidget *widget, gpointer user_data) {
    BridgeApp *app = (BridgeApp *)user_data;
    
    // Update settings from UI
    if (widget == app->auto_start_check) {
        app->auto_start = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->auto_start_check));
    } else if (widget == app->verbose_logging_check) {
        app->verbose_logging = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->verbose_logging_check));
    } else if (widget == app->device_permissions_combo) {
        int active = gtk_combo_box_get_active(GTK_COMBO_BOX(app->device_permissions_combo));
        if (app->device_permissions) g_free(app->device_permissions);

        switch (active) {
            case 1: app->device_permissions = g_strdup("666"); break;
            case 2: app->device_permissions = g_strdup("644"); break;
            case 3: app->device_permissions = g_strdup("600"); break;
            default: app->device_permissions = g_strdup(""); break;
        }
    }
    
    // Save settings
    save_settings(app);
}

// Thread functions for non-blocking operations
gpointer start_nullmodem_thread(gpointer data) {
    BridgeApp *app = (BridgeApp *)data;
    
    gboolean success = create_null_modem(app);
    
    // Update UI from main thread
    g_idle_add((GSourceFunc)update_ui_state, app);
    
    if (!success) {
        g_idle_add((GSourceFunc)show_error_dialog, "Failed to start null modem. Check log for details.");
    }
    
    return NULL;
}

gpointer stop_nullmodem_thread(gpointer data) {
    BridgeApp *app = (BridgeApp *)data;
    
    stop_null_modem(app);
    
    // Update UI from main thread
    g_idle_add((GSourceFunc)update_ui_state, app);
    
    return NULL;
}

gpointer test_communication_thread(gpointer data) {
    BridgeApp *app = (BridgeApp *)data;
    
    gboolean success = test_null_modem_communication(app);
    
    // Show result dialog from main thread
    if (success) {
        g_idle_add((GSourceFunc)show_info_dialog, "✓ Communication test passed!");
    } else {
        g_idle_add((GSourceFunc)show_error_dialog, "✗ Communication test failed!");
    }
    
    return NULL;
}

gboolean show_error_dialog(gpointer data) {
    const char *message = (const char *)data;
    
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK,
                                              "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    return FALSE; // Remove from idle queue
}

gboolean show_info_dialog(gpointer data) {
    const char *message = (const char *)data;

    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_INFO,
                                              GTK_BUTTONS_OK,
                                              "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return FALSE; // Remove from idle queue
}

// Sniffing callback implementations
void on_sniffing_enable_toggled(GtkToggleButton *button, gpointer user_data) {
    BridgeApp *app = (BridgeApp *)user_data;
    app->sniffing_enabled = gtk_toggle_button_get_active(button);

    // Enable/disable sniffing controls based on checkbox state
    gtk_widget_set_sensitive(app->sniff_start_button, app->sniffing_enabled && app->state == BRIDGE_STATE_RUNNING);

    log_message(app, "Sniffing %s", app->sniffing_enabled ? "enabled" : "disabled");
}

void on_sniff_start_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    BridgeApp *app = (BridgeApp *)user_data;

    // Update sniffing settings from UI
    update_sniff_settings_from_ui(app);

    if (start_sniffing(app)) {
        gtk_widget_set_sensitive(app->sniff_start_button, FALSE);
        gtk_widget_set_sensitive(app->sniff_stop_button, TRUE);
    }
}

void on_sniff_stop_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    BridgeApp *app = (BridgeApp *)user_data;

    stop_sniffing(app);
    gtk_widget_set_sensitive(app->sniff_start_button, TRUE);
    gtk_widget_set_sensitive(app->sniff_stop_button, FALSE);

    // Update statistics display
    gtk_label_set_text(GTK_LABEL(app->sniff_stats_label), "Sniffing stopped");
}

void on_sniff_output_toggled(GtkToggleButton *button, gpointer user_data) {
    (void)button;
    BridgeApp *app = (BridgeApp *)user_data;

    // Update output methods from checkboxes
    app->sniff_output_methods = SNIFF_OUTPUT_NONE;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_pipe_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_PIPE;
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_tcp_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_TCP;
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_udp_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_UDP;
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_file_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_FILE;
    }
}

void on_sniff_settings_changed(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    BridgeApp *app = (BridgeApp *)user_data;

    // Update settings from UI controls
    update_sniff_settings_from_ui(app);
}

void update_sniff_settings_from_ui(BridgeApp *app) {
    // Update output methods
    app->sniff_output_methods = SNIFF_OUTPUT_NONE;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_pipe_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_PIPE;
        const char *pipe_path = gtk_entry_get_text(GTK_ENTRY(app->sniff_pipe_entry));
        strncpy(app->sniff_pipe_path, pipe_path, MAX_PATH_LENGTH - 1);
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_tcp_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_TCP;
        const char *tcp_port = gtk_entry_get_text(GTK_ENTRY(app->sniff_tcp_port_entry));
        app->sniff_tcp_port = atoi(tcp_port);
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_udp_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_UDP;
        const char *udp_addr = gtk_entry_get_text(GTK_ENTRY(app->sniff_udp_addr_entry));
        const char *udp_port = gtk_entry_get_text(GTK_ENTRY(app->sniff_udp_port_entry));
        strncpy(app->sniff_udp_addr, udp_addr, 63);
        app->sniff_udp_port = atoi(udp_port);
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->sniff_file_check))) {
        app->sniff_output_methods |= SNIFF_OUTPUT_FILE;
        const char *log_file = gtk_entry_get_text(GTK_ENTRY(app->sniff_file_entry));
        strncpy(app->sniff_log_file, log_file, MAX_PATH_LENGTH - 1);
    }

    // Update direction setting
    int direction_index = gtk_combo_box_get_active(GTK_COMBO_BOX(app->sniff_direction_combo));
    switch (direction_index) {
        case 0: app->sniff_direction = SNIFF_DIRECTION_BOTH; break;
        case 1: app->sniff_direction = SNIFF_DIRECTION_RX_ONLY; break;
        case 2: app->sniff_direction = SNIFF_DIRECTION_TX_ONLY; break;
        default: app->sniff_direction = SNIFF_DIRECTION_BOTH; break;
    }

    // Update format setting
    int format_index = gtk_combo_box_get_active(GTK_COMBO_BOX(app->sniff_format_combo));
    switch (format_index) {
        case 0: app->sniff_format = SNIFF_FORMAT_RAW; break;
        case 1: app->sniff_format = SNIFF_FORMAT_HEX; break;
        case 2: app->sniff_format = SNIFF_FORMAT_TEXT; break;
        default: app->sniff_format = SNIFF_FORMAT_HEX; break;
    }
}
