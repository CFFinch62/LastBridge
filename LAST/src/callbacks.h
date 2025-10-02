#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "common.h"

/*
 * Event handlers module for LAST
 * Contains all GTK callback functions
 */

// Connection callbacks
void on_connect_clicked(GtkWidget *widget, gpointer data);
void on_disconnect_clicked(GtkWidget *widget, gpointer data);
void on_refresh_clicked(GtkWidget *widget, gpointer data);
void on_connection_setting_changed(GtkWidget *widget, gpointer data);

// Data transmission callbacks
void on_send_activate(GtkWidget *widget, gpointer data);
void on_send_clicked(GtkWidget *widget, gpointer data);

// Data management callbacks
void on_clear_clicked(GtkWidget *widget, gpointer data);
void on_save_clicked(GtkWidget *widget, gpointer data);

// File operation callbacks
void on_send_file_clicked(GtkWidget *widget, gpointer data);
void on_send_file_stop_clicked(GtkWidget *widget, gpointer data);
void on_log_toggled(GtkWidget *widget, gpointer data);

// Control signal callbacks
void on_dtr_toggled(GtkWidget *widget, gpointer data);
void on_rts_toggled(GtkWidget *widget, gpointer data);
void on_break_clicked(GtkWidget *widget, gpointer data);

// Display option callbacks
void on_hex_display_toggled(GtkWidget *widget, gpointer data);
void on_hex_bytes_per_line_changed(GtkWidget *widget, gpointer data);
void on_timestamp_toggled(GtkWidget *widget, gpointer data);
void on_autoscroll_toggled(GtkWidget *widget, gpointer data);
void on_local_echo_toggled(GtkWidget *widget, gpointer data);
void on_line_ending_changed(GtkWidget *widget, gpointer data);

// Appearance callbacks
void on_theme_changed(GtkWidget *widget, gpointer data);
void on_font_changed(GtkWidget *widget, gpointer data);
void on_bg_color_changed(GtkWidget *widget, gpointer data);
void on_text_color_changed(GtkWidget *widget, gpointer data);

// Application lifecycle callbacks
void on_window_destroy(GtkWidget *widget, gpointer data);

// Menu callbacks
void on_file_exit_activate(GtkWidget *widget, gpointer data);
void on_tools_bridge_activate(GtkWidget *widget, gpointer data);
void on_help_about_activate(GtkWidget *widget, gpointer data);

// View menu callbacks
void on_view_appearance_activate(GtkWidget *widget, gpointer data);
void on_view_display_options_activate(GtkWidget *widget, gpointer data);

// Function to connect all signals
void connect_signals(SerialTerminal *terminal);

#endif // CALLBACKS_H
