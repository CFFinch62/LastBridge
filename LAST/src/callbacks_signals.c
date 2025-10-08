/*
 * Signal connection for LAST - Linux Advanced Serial Transceiver
 * Connects all GTK signals to their callback functions
 */

#include "callbacks.h"

// Function to connect all signals
void connect_signals(SerialTerminal *terminal) {
    // Connection signals
    g_signal_connect(terminal->connection_type_combo, "changed", G_CALLBACK(on_connection_type_changed), terminal);
    g_signal_connect(terminal->connect_button, "clicked", G_CALLBACK(on_connect_clicked), terminal);
    g_signal_connect(terminal->disconnect_button, "clicked", G_CALLBACK(on_disconnect_clicked), terminal);
    g_signal_connect(terminal->refresh_button, "clicked", G_CALLBACK(on_refresh_clicked), terminal);

    // Connection setting signals
    g_signal_connect(terminal->port_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->baudrate_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->databits_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->parity_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->stopbits_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->flowcontrol_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);

    // Data transmission signals
    g_signal_connect(terminal->send_entry, "activate", G_CALLBACK(on_send_activate), terminal);
    g_signal_connect(terminal->send_button, "clicked", G_CALLBACK(on_send_clicked), terminal);

    // Data management signals
    g_signal_connect(terminal->clear_button, "clicked", G_CALLBACK(on_clear_clicked), terminal);
    g_signal_connect(terminal->save_button, "clicked", G_CALLBACK(on_save_clicked), terminal);

    // File operation signals
    g_signal_connect(terminal->send_file_button, "clicked", G_CALLBACK(on_send_file_clicked), terminal);
    g_signal_connect(terminal->send_file_stop_button, "clicked", G_CALLBACK(on_send_file_stop_clicked), terminal);
    g_signal_connect(terminal->send_file_lines_check, "toggled", G_CALLBACK(on_send_file_lines_toggled), terminal);
    g_signal_connect(terminal->log_file_button, "toggled", G_CALLBACK(on_log_toggled), terminal);

    // Control signal signals
    g_signal_connect(terminal->dtr_check, "toggled", G_CALLBACK(on_dtr_toggled), terminal);
    g_signal_connect(terminal->rts_check, "toggled", G_CALLBACK(on_rts_toggled), terminal);
    g_signal_connect(terminal->break_button, "clicked", G_CALLBACK(on_break_clicked), terminal);

    // Display option signals
    g_signal_connect(terminal->hex_display_check, "toggled", G_CALLBACK(on_hex_display_toggled), terminal);
    g_signal_connect(terminal->hex_bytes_per_line_combo, "changed", G_CALLBACK(on_hex_bytes_per_line_changed), terminal);
    g_signal_connect(terminal->timestamp_check, "toggled", G_CALLBACK(on_timestamp_toggled), terminal);
    g_signal_connect(terminal->autoscroll_check, "toggled", G_CALLBACK(on_autoscroll_toggled), terminal);
    g_signal_connect(terminal->local_echo_check, "toggled", G_CALLBACK(on_local_echo_toggled), terminal);
    g_signal_connect(terminal->line_ending_combo, "changed", G_CALLBACK(on_line_ending_changed), terminal);

    // Appearance signals
    g_signal_connect(terminal->theme_combo, "changed", G_CALLBACK(on_theme_changed), terminal);
    g_signal_connect(terminal->font_button, "font-set", G_CALLBACK(on_font_changed), terminal);
    g_signal_connect(terminal->bg_color_button, "color-set", G_CALLBACK(on_bg_color_changed), terminal);
    g_signal_connect(terminal->text_color_button, "color-set", G_CALLBACK(on_text_color_changed), terminal);

    // Application lifecycle signals
    g_signal_connect(terminal->window, "destroy", G_CALLBACK(on_window_destroy), terminal);
}

