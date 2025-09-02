/*
 * Utility functions module for BRIDGE - Virtual Null Modem Bridge
 * Logging, formatting, and helper functions
 */

#include "utils.h"
#include "ui.h"

char* get_current_timestamp(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char *timestamp = malloc(32);
    strftime(timestamp, 32, "%Y-%m-%d %H:%M:%S", tm_info);
    return timestamp;
}

void log_message(BridgeApp *app, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    char message[MAX_LOG_LENGTH];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Print to console if verbose logging is enabled
    if (app->verbose_logging) {
        char *timestamp = get_current_timestamp();
        printf("[%s] %s\n", timestamp, message);
        free(timestamp);
    }
    
    // Add to GUI log
    append_log_message(app, message, TRUE);
}

gboolean update_status_timer(gpointer data) {
    BridgeApp *app = (BridgeApp *)data;
    
    // Update connection time display
    if (app->connection_time_label && app->state == BRIDGE_STATE_RUNNING) {
        char time_buffer[64];
        format_connection_time(app, time_buffer, sizeof(time_buffer));
        gtk_label_set_text(GTK_LABEL(app->connection_time_label), time_buffer);
    }
    
    // Update status if needed
    update_ui_state(app);
    
    return TRUE; // Continue timer
}

void format_connection_time(BridgeApp *app, char *buffer, size_t buffer_size) {
    if (app->state != BRIDGE_STATE_RUNNING) {
        snprintf(buffer, buffer_size, "Not running");
        return;
    }
    
    format_uptime(app->start_time, buffer, buffer_size);
}

void format_uptime(time_t start_time, char *buffer, size_t buffer_size) {
    time_t current_time = time(NULL);
    int duration = (int)(current_time - start_time);
    
    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;
    int seconds = duration % 60;
    
    snprintf(buffer, buffer_size, "%02d:%02d:%02d", hours, minutes, seconds);
}

gboolean file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

gboolean is_process_running(pid_t pid) {
    if (pid <= 0) {
        return FALSE;
    }
    
    // Send signal 0 to check if process exists
    return (kill(pid, 0) == 0);
}

void safe_kill_process(pid_t pid) {
    if (pid <= 0) {
        return;
    }
    
    // Try SIGTERM first
    if (kill(pid, SIGTERM) == 0) {
        // Wait a bit for graceful shutdown
        for (int i = 0; i < 50; i++) { // Wait up to 5 seconds
            if (!is_process_running(pid)) {
                return; // Process terminated gracefully
            }
            usleep(100000); // 100ms
        }
        
        // If still running, use SIGKILL
        kill(pid, SIGKILL);
        
        // Wait for process to be reaped
        int status;
        waitpid(pid, &status, 0);
    }
}
