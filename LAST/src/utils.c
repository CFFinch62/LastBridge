/*
 * Utility functions module for LAST - Linux Advanced Serial Transceiver
 * Handles data formatting, timestamps, and statistics
 */

#include "utils.h"

char* format_data_for_display(const char *data, gboolean hex_mode) {
    if (!hex_mode) {
        return strdup(data);
    }

    // Convert to hex display
    size_t len = strlen(data);
    char *hex_str = malloc(len * 3 + 1); // Each byte becomes "XX "
    hex_str[0] = '\0';

    for (size_t i = 0; i < len; i++) {
        char hex_byte[4];
        snprintf(hex_byte, sizeof(hex_byte), "%02X ", (unsigned char)data[i]);
        strcat(hex_str, hex_byte);
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
