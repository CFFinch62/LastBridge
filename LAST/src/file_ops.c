/*
 * File operations module for LAST - Linux Advanced Serial Transceiver
 * Handles file sending, saving, and logging
 */

#include "file_ops.h"
#include "serial.h"
#include "utils.h"

void clear_receive_area(SerialTerminal *terminal) {
    // Clear text display
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->receive_text));
    gtk_text_buffer_set_text(text_buffer, "", -1);

    // Clear hex display if it exists
    if (terminal->hex_text) {
        GtkTextBuffer *hex_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->hex_text));
        gtk_text_buffer_set_text(hex_buffer, "", -1);
    }
}

void save_received_data(SerialTerminal *terminal) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Received Data",
                                                   GTK_WINDOW(terminal->window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);

    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "serial_data.txt");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Get text data
        GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->receive_text));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(text_buffer, &start, &end);
        char *text_data = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);

        FILE *file = fopen(filename, "w");
        if (file) {
            // Always save text data
            fprintf(file, "=== TEXT DATA ===\n");
            fprintf(file, "%s", text_data);

            // If hex display is enabled and has data, save hex data too
            if (terminal->hex_display && terminal->hex_text) {
                GtkTextBuffer *hex_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->hex_text));
                GtkTextIter hex_start, hex_end;
                gtk_text_buffer_get_bounds(hex_buffer, &hex_start, &hex_end);
                char *hex_data = gtk_text_buffer_get_text(hex_buffer, &hex_start, &hex_end, FALSE);

                if (strlen(hex_data) > 0) {
                    fprintf(file, "\n\n=== HEX DATA ===\n");
                    fprintf(file, "%s", hex_data);
                }

                g_free(hex_data);
            }

            fclose(file);

            char status_msg[512];
            snprintf(status_msg, sizeof(status_msg), "Data saved to %s", filename);
            gtk_label_set_text(GTK_LABEL(terminal->status_label), status_msg);
        }

        g_free(text_data);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void send_file(SerialTerminal *terminal) {
    if (!terminal->connected) return;

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Send File",
                                                   GTK_WINDOW(terminal->window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Check which modes are enabled
        gboolean repeat_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->send_file_repeat_check));
        gboolean lines_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->send_file_lines_check));

        // Store filename for any mode that needs it
        if (repeat_mode || lines_mode) {
            if (terminal->repeat_filename) {
                free(terminal->repeat_filename);
            }
            terminal->repeat_filename = strdup(filename);
        }

        // Get interval from combo box
        const char *interval_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo));

        if (lines_mode) {
            // Line-by-line mode (with or without repeat)
            terminal->line_by_line_delay_ms = atoi(interval_str);
            terminal->line_by_line_mode = repeat_mode; // TRUE if both repeat and lines are checked
            send_file_line_by_line(terminal, filename);
        } else if (repeat_mode) {
            // Traditional repeat mode (whole file)
            terminal->repeat_interval = atof(interval_str);
            start_repeat_file_sending(terminal);
        } else {
            // Send file once, normally
            send_file_once(terminal, filename);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void send_file_once(SerialTerminal *terminal, const char *filename) {
    if (!terminal->connected) return;

    FILE *file = fopen(filename, "r");
    if (file) {
        char buffer[1024];
        size_t total_sent = 0;

        while (fgets(buffer, sizeof(buffer), file)) {
            size_t len = strlen(buffer);
            ssize_t bytes_written = write(terminal->connection_fd, buffer, len);
            if (bytes_written > 0) {
                terminal->bytes_sent += bytes_written;
                total_sent += bytes_written;
            }
            usleep(10000); // 10ms delay between lines
        }

        fclose(file);

        char status_msg[512];
        snprintf(status_msg, sizeof(status_msg), "File sent: %zu bytes from %s", total_sent, filename);
        show_status_message(terminal, status_msg);
    } else {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "Error: Could not open file %s", filename);
        show_status_message(terminal, error_msg);
    }
}

void start_repeat_file_sending(SerialTerminal *terminal) {
    // Stop any existing repeat timer
    stop_repeat_file_sending(terminal);

    terminal->repeat_file_sending = TRUE;

    // Convert interval to milliseconds
    guint interval_ms = (guint)(terminal->repeat_interval * 1000);

    // Start timer
    terminal->repeat_timer_id = g_timeout_add(interval_ms, repeat_file_timer_callback, terminal);

    // Update UI
    gtk_widget_set_sensitive(terminal->send_file_button, FALSE);
    gtk_widget_set_sensitive(terminal->send_file_stop_button, TRUE);

    char status_msg[512];
    snprintf(status_msg, sizeof(status_msg), "Started repeating file %s every %.1f seconds",
             terminal->repeat_filename, terminal->repeat_interval);
    append_to_receive_text(terminal, status_msg, FALSE);

    // Send file immediately for the first time
    send_file_once(terminal, terminal->repeat_filename);
}

void stop_repeat_file_sending(SerialTerminal *terminal) {
    // Stop traditional repeat sending
    if (terminal->repeat_file_sending && terminal->repeat_timer_id > 0) {
        g_source_remove(terminal->repeat_timer_id);
        terminal->repeat_timer_id = 0;
    }
    terminal->repeat_file_sending = FALSE;

    // Stop line-by-line sending
    stop_line_by_line_sending(terminal);

    // Update UI (only if not already updated by stop_line_by_line_sending)
    if (!terminal->line_by_line_sending) {
        gtk_widget_set_sensitive(terminal->send_file_button, TRUE);
        gtk_widget_set_sensitive(terminal->send_file_stop_button, FALSE);

        if (terminal->repeat_filename) {
            char status_msg[512];
            snprintf(status_msg, sizeof(status_msg), "Stopped file sending: %s", terminal->repeat_filename);
            show_status_message(terminal, status_msg);
        }
    }
}

gboolean repeat_file_timer_callback(gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;

    if (!terminal->connected || !terminal->repeat_file_sending) {
        stop_repeat_file_sending(terminal);
        return FALSE; // Stop timer
    }

    if (terminal->repeat_filename) {
        send_file_once(terminal, terminal->repeat_filename);
    }

    return TRUE; // Continue timer
}

void toggle_logging(SerialTerminal *terminal) {
    gboolean logging = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->log_file_button));

    if (logging) {
        const char *filename = gtk_entry_get_text(GTK_ENTRY(terminal->log_file_entry));
        if (strlen(filename) == 0) {
            // Open file chooser
            GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Log File",
                                                           GTK_WINDOW(terminal->window),
                                                           GTK_FILE_CHOOSER_ACTION_SAVE,
                                                           "_Cancel", GTK_RESPONSE_CANCEL,
                                                           "_Save", GTK_RESPONSE_ACCEPT,
                                                           NULL);

            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "serial_log.txt");

            if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
                char *selected_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
                gtk_entry_set_text(GTK_ENTRY(terminal->log_file_entry), selected_filename);
                filename = selected_filename;
                g_free(selected_filename);
            } else {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->log_file_button), FALSE);
                gtk_widget_destroy(dialog);
                return;
            }

            gtk_widget_destroy(dialog);
        }

        terminal->log_file = fopen(filename, "a");
        if (terminal->log_file) {
            char *timestamp = get_current_timestamp();
            fprintf(terminal->log_file, "\n=== Log started at %s ===\n", timestamp);
            fflush(terminal->log_file);
            free(timestamp);

            if (terminal->log_filename) free(terminal->log_filename);
            terminal->log_filename = strdup(filename);
        } else {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->log_file_button), FALSE);
        }
    } else {
        if (terminal->log_file) {
            char *timestamp = get_current_timestamp();
            fprintf(terminal->log_file, "=== Log ended at %s ===\n\n", timestamp);
            fclose(terminal->log_file);
            terminal->log_file = NULL;
            free(timestamp);
        }
    }
}

void send_file_line_by_line(SerialTerminal *terminal, const char *filename) {
    if (!terminal->connected) return;

    // Stop any existing line-by-line sending (silently)
    if (terminal->line_by_line_sending && terminal->line_by_line_timer_id > 0) {
        g_source_remove(terminal->line_by_line_timer_id);
        terminal->line_by_line_timer_id = 0;
    }
    if (terminal->line_by_line_file) {
        fclose(terminal->line_by_line_file);
        terminal->line_by_line_file = NULL;
    }
    terminal->line_by_line_sending = FALSE;

    // Open the file
    terminal->line_by_line_file = fopen(filename, "r");
    if (!terminal->line_by_line_file) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "Error: Could not open file %s", filename);
        append_to_receive_text(terminal, error_msg, FALSE);
        return;
    }

    // Initialize line-by-line state
    terminal->line_by_line_sending = TRUE;
    terminal->current_line_number = 0;

    // Update UI
    gtk_widget_set_sensitive(terminal->send_file_button, FALSE);
    gtk_widget_set_sensitive(terminal->send_file_stop_button, TRUE);

    char status_msg[512];
    if (terminal->line_by_line_mode) {
        snprintf(status_msg, sizeof(status_msg), "Started line-by-line repeat sending of %s with %dms delay",
                 filename, terminal->line_by_line_delay_ms);
    } else {
        snprintf(status_msg, sizeof(status_msg), "Started line-by-line sending of %s with %dms delay",
                 filename, terminal->line_by_line_delay_ms);
    }
    show_status_message(terminal, status_msg);

    // Start the timer to send the first line immediately
    terminal->line_by_line_timer_id = g_timeout_add(1, line_by_line_timer_callback, terminal);
}

void start_line_by_line_sending(SerialTerminal *terminal) {
    if (terminal->repeat_filename) {
        send_file_line_by_line(terminal, terminal->repeat_filename);
    }
}

void stop_line_by_line_sending(SerialTerminal *terminal) {
    if (terminal->line_by_line_sending && terminal->line_by_line_timer_id > 0) {
        g_source_remove(terminal->line_by_line_timer_id);
        terminal->line_by_line_timer_id = 0;
    }

    if (terminal->line_by_line_file) {
        fclose(terminal->line_by_line_file);
        terminal->line_by_line_file = NULL;
    }

    terminal->line_by_line_sending = FALSE;

    // Update UI
    gtk_widget_set_sensitive(terminal->send_file_button, TRUE);
    gtk_widget_set_sensitive(terminal->send_file_stop_button, FALSE);

    if (terminal->repeat_filename) {
        char status_msg[512];
        snprintf(status_msg, sizeof(status_msg), "Stopped line-by-line sending of %s", terminal->repeat_filename);
        show_status_message(terminal, status_msg);
    }
}

gboolean line_by_line_timer_callback(gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;

    if (!terminal->connected || !terminal->line_by_line_sending || !terminal->line_by_line_file) {
        stop_line_by_line_sending(terminal);
        return FALSE; // Stop timer
    }

    char line[1024];
    if (fgets(line, sizeof(line), terminal->line_by_line_file)) {
        // Remove any existing line endings from the line
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
            len--;
        }

        // Send the line content
        ssize_t bytes_written = 0;
        if (len > 0) {
            bytes_written = write(terminal->connection_fd, line, len);
            if (bytes_written > 0) {
                terminal->bytes_sent += bytes_written;
                // Mark TX activity
                terminal->tx_active = TRUE;
                terminal->tx_last_activity = time(NULL);
            }
        }

        // Always add CR-LF line ending
        const char *line_ending = "\r\n";
        ssize_t ending_bytes = write(terminal->connection_fd, line_ending, strlen(line_ending));
        if (ending_bytes > 0) {
            terminal->bytes_sent += ending_bytes;
            // Mark TX activity for line ending too
            terminal->tx_active = TRUE;
            terminal->tx_last_activity = time(NULL);
        }

        terminal->current_line_number++;

        // Log to file if enabled
        if (terminal->log_file) {
            char *timestamp = get_current_timestamp();
            fprintf(terminal->log_file, "[%s] TX: %s\n", timestamp, line);
            fflush(terminal->log_file);
            free(timestamp);
        }

        // Local echo if enabled
        if (terminal->local_echo) {
            char echo_text[1024];
            snprintf(echo_text, sizeof(echo_text), "TX: %s", line);
            append_to_receive_text(terminal, echo_text, FALSE);
        }

        // Force GUI updates to be processed immediately for real-time display
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }

        // Schedule next line
        terminal->line_by_line_timer_id = g_timeout_add(terminal->line_by_line_delay_ms,
                                                       line_by_line_timer_callback, terminal);
        return FALSE; // Stop current timer (new one was scheduled)
    } else {
        // End of file reached
        if (terminal->line_by_line_mode) {
            // Repeat mode: restart from beginning
            fclose(terminal->line_by_line_file);
            terminal->line_by_line_file = fopen(terminal->repeat_filename, "r");
            if (terminal->line_by_line_file) {
                terminal->current_line_number = 0;
                // Schedule next line (first line of repeated file)
                terminal->line_by_line_timer_id = g_timeout_add(terminal->line_by_line_delay_ms,
                                                               line_by_line_timer_callback, terminal);
                return FALSE; // Stop current timer (new one was scheduled)
            }
        }

        // Either not in repeat mode, or failed to reopen file
        char status_msg[512];
        snprintf(status_msg, sizeof(status_msg), "Completed line-by-line sending: %d lines sent",
                 terminal->current_line_number);
        append_to_receive_text(terminal, status_msg, FALSE);

        stop_line_by_line_sending(terminal);
        return FALSE; // Stop timer
    }
}
