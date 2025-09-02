/*
 * Callbacks module header for BRIDGE - Virtual Null Modem Bridge
 * GTK event handlers and signal connections
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "common.h"

// Function declarations
void connect_signals(BridgeApp *app);
void on_start_button_clicked(GtkButton *button, gpointer user_data);
void on_stop_button_clicked(GtkButton *button, gpointer user_data);
void on_test_button_clicked(GtkButton *button, gpointer user_data);
void on_clear_log_clicked(GtkButton *button, gpointer user_data);
void on_window_destroy(GtkWidget *widget, gpointer user_data);
void on_device_entry_changed(GtkEntry *entry, gpointer user_data);
void on_settings_changed(GtkWidget *widget, gpointer user_data);

// Thread functions
gpointer start_nullmodem_thread(gpointer data);
gpointer stop_nullmodem_thread(gpointer data);
gpointer test_communication_thread(gpointer data);

// Dialog functions
gboolean show_error_dialog(gpointer data);
gboolean show_info_dialog(gpointer data);

#endif // CALLBACKS_H
