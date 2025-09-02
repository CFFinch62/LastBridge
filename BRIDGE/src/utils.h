/*
 * Utility functions module header for BRIDGE - Virtual Null Modem Bridge
 * Logging, formatting, and helper functions
 */

#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include <stdarg.h>

// Function declarations
char* get_current_timestamp(void);
void log_message(BridgeApp *app, const char *format, ...);
gboolean update_status_timer(gpointer data);
void format_connection_time(BridgeApp *app, char *buffer, size_t buffer_size);
void format_uptime(time_t start_time, char *buffer, size_t buffer_size);
gboolean file_exists(const char *path);
gboolean is_process_running(pid_t pid);
void safe_kill_process(pid_t pid);

#endif // UTILS_H
