/*
 * Utility functions module for LAST - Linux Advanced Serial Transceiver
 * Handles data formatting, timestamps, and statistics
 */

#include "utils.h"

char* format_data_for_display(const char *data, size_t data_len, gboolean hex_mode) {
    if (!hex_mode) {
        // In text mode, create a null-terminated copy of the data
        char *text_copy = malloc(data_len + 1);
        memcpy(text_copy, data, data_len);
        text_copy[data_len] = '\0';
        return text_copy;
    }

    // Get hex bytes per line setting from global terminal (extern in common.h)
    extern SerialTerminal *g_terminal;
    int bytes_per_line = (g_terminal && g_terminal->hex_bytes_per_line > 0) ?
                         g_terminal->hex_bytes_per_line : 0;

    // Convert to hex display
    // Allocate extra space for potential newlines and null terminator
    char *hex_str = malloc(data_len * 4 + 100); // Extra space for newlines and formatting
    hex_str[0] = '\0';

    if (bytes_per_line == 0) {
        // Mode 1: Line breaks at CR LF sequences (0D 0A)
        for (size_t i = 0; i < data_len; i++) {
            char hex_byte[4];
            unsigned char byte = (unsigned char)data[i];
            snprintf(hex_byte, sizeof(hex_byte), "%02X ", byte);
            strcat(hex_str, hex_byte);

            // Check for CR LF sequence (0D 0A) to create line breaks
            if (byte == 0x0D && i + 1 < data_len && (unsigned char)data[i + 1] == 0x0A) {
                // Add the LF byte
                i++;
                snprintf(hex_byte, sizeof(hex_byte), "%02X", (unsigned char)data[i]);
                strcat(hex_str, hex_byte);
                // Add a newline to display for better readability
                strcat(hex_str, "\n");
            }
        }
    } else {
        // Mode 2: Fixed number of bytes per line
        for (size_t i = 0; i < data_len; i++) {
            char hex_byte[4];
            unsigned char byte = (unsigned char)data[i];
            snprintf(hex_byte, sizeof(hex_byte), "%02X ", byte);
            strcat(hex_str, hex_byte);

            // Add newline after specified number of bytes
            if ((i + 1) % bytes_per_line == 0) {
                strcat(hex_str, "\n");
            }
        }
    }

    return hex_str;
}

char* get_current_timestamp(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char *timestamp = malloc(32);
    strftime(timestamp, 32, "%Y-%m-%d %H:%M:%S", tm_info);
    return timestamp;
}

void update_statistics(SerialTerminal *terminal) {
    if (!terminal->stats_label) return;

    char stats_text[256];

    if (terminal->connected) {
        time_t current_time = time(NULL);
        int connection_duration = (int)(current_time - terminal->connection_start_time);
        int hours = connection_duration / 3600;
        int minutes = (connection_duration % 3600) / 60;
        int seconds = connection_duration % 60;

        snprintf(stats_text, sizeof(stats_text),
                "Sent: %lu bytes | Received: %lu bytes | Time: %02d:%02d:%02d",
                terminal->bytes_sent, terminal->bytes_received, hours, minutes, seconds);
    } else {
        snprintf(stats_text, sizeof(stats_text),
                "Sent: %lu bytes | Received: %lu bytes | Time: 00:00:00",
                terminal->bytes_sent, terminal->bytes_received);
    }

    gtk_label_set_text(GTK_LABEL(terminal->stats_label), stats_text);
}

gboolean update_statistics_timer(gpointer data) {
    update_statistics((SerialTerminal *)data);
    return TRUE; // Continue timer
}
