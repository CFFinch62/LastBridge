#ifndef FILE_OPS_H
#define FILE_OPS_H

#include "common.h"

/*
 * File operations module for LAST
 * Handles file sending, saving, and logging
 */

// Data management functions
void clear_receive_area(SerialTerminal *terminal);
void save_received_data(SerialTerminal *terminal);

// File sending functions
void send_file(SerialTerminal *terminal);
void send_file_once(SerialTerminal *terminal, const char *filename);

// Repeat file sending functions
void start_repeat_file_sending(SerialTerminal *terminal);
void stop_repeat_file_sending(SerialTerminal *terminal);
gboolean repeat_file_timer_callback(gpointer data);

// Line-by-line file sending functions
void send_file_line_by_line(SerialTerminal *terminal, const char *filename);
void start_line_by_line_sending(SerialTerminal *terminal);
void stop_line_by_line_sending(SerialTerminal *terminal);
gboolean line_by_line_timer_callback(gpointer data);

// Logging functions
void toggle_logging(SerialTerminal *terminal);

#endif // FILE_OPS_H
