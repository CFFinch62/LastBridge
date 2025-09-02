#ifndef SERIAL_H
#define SERIAL_H

#include "common.h"

// Forward declarations for functions from other modules
char* get_current_timestamp(void);
char* format_data_for_display(const char *data, gboolean hex_mode);
void stop_repeat_file_sending(SerialTerminal *terminal);

/*
 * Serial communication module for LAST
 * Handles port detection, connection, configuration, and I/O
 */

// Port scanning and detection functions
void scan_all_serial_devices(GtkComboBoxText *combo);
gboolean is_serial_device(const char *path);

// Connection management functions
void connect_serial(SerialTerminal *terminal);
void disconnect_serial(SerialTerminal *terminal);

// Serial configuration functions
void apply_serial_settings(SerialTerminal *terminal);

// Data I/O functions
void send_data(SerialTerminal *terminal);
void *read_thread_func(void *arg);

// Text display functions
void append_to_receive_text(SerialTerminal *terminal, const char *text, gboolean is_received);
gboolean append_to_receive_text_idle(gpointer data);

// Control signal functions
void set_control_signals(SerialTerminal *terminal);
void send_break_signal(SerialTerminal *terminal);

#endif // SERIAL_H
