/*
 * Data transmission callbacks for LAST - Linux Advanced Serial Transceiver
 * Handles data send, clear, save, file operations, logging, and control signals
 */

#include "callbacks.h"
#include "serial.h"
#include "file_ops.h"
#include "ui.h"

// Data transmission callbacks
void on_send_activate(GtkWidget *widget, gpointer data) {
    (void)widget;
    send_data((SerialTerminal *)data);
}

void on_send_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    send_data((SerialTerminal *)data);
}

// Data management callbacks
void on_clear_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    clear_receive_area((SerialTerminal *)data);
}

void on_save_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    save_received_data((SerialTerminal *)data);
}

// File operation callbacks
void on_send_file_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    send_file((SerialTerminal *)data);
}

void on_send_file_stop_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    stop_repeat_file_sending((SerialTerminal *)data);
}

void on_send_file_lines_toggled(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    update_interval_dropdown_based_on_mode(terminal);
}

void on_log_toggled(GtkWidget *widget, gpointer data) {
    (void)widget;
    toggle_logging((SerialTerminal *)data);
}

// Control signal callbacks
void on_dtr_toggled(GtkWidget *widget, gpointer data) {
    (void)widget;
    set_control_signals((SerialTerminal *)data);
}

void on_rts_toggled(GtkWidget *widget, gpointer data) {
    (void)widget;
    set_control_signals((SerialTerminal *)data);
}

void on_break_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    send_break_signal((SerialTerminal *)data);
}

