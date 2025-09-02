/*
 * Callbacks module for BRIDGE - Virtual Null Modem Bridge
 * GTK event handlers and signal connections
 */

#include "callbacks.h"
#include "nullmodem.h"
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
