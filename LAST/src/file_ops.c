/*
 * File operations module for LAST - Linux Advanced Serial Transceiver
 * Handles file sending, saving, and logging
 */

#include "file_ops.h"
#include "serial.h"
#include "utils.h"

void clear_receive_area(SerialTerminal *terminal) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->receive_text));
    gtk_text_buffer_set_text(buffer, "", -1);
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

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->receive_text));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        FILE *file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", text);
            fclose(file);

            char status_msg[512];
            snprintf(status_msg, sizeof(status_msg), "Data saved to %s", filename);
            gtk_label_set_text(GTK_LABEL(terminal->status_label), status_msg);
        }

        g_free(text);
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

        // Check if repeat mode is enabled
        gboolean repeat_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->send_file_repeat_check));

        if (repeat_mode) {
            // Store filename for repeated sending
            if (terminal->repeat_filename) {
                free(terminal->repeat_filename);
            }
            terminal->repeat_filename = strdup(filename);

            // Get interval from combo box
            const char *interval_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo));
            terminal->repeat_interval = atof(interval_str);

            // Start repeat sending
            start_repeat_file_sending(terminal);
        } else {
            // Send file once
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
            ssize_t bytes_written = write(terminal->serial_fd, buffer, len);
            if (bytes_written > 0) {
                terminal->bytes_sent += bytes_written;
                total_sent += bytes_written;
            }
            usleep(10000); // 10ms delay between lines
        }

        fclose(file);

        char status_msg[512];
        snprintf(status_msg, sizeof(status_msg), "File sent: %zu bytes from %s", total_sent, filename);
        append_to_receive_text(terminal, status_msg, FALSE);
    } else {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "Error: Could not open file %s", filename);
        append_to_receive_text(terminal, error_msg, FALSE);
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
    if (terminal->repeat_file_sending && terminal->repeat_timer_id > 0) {
        g_source_remove(terminal->repeat_timer_id);
        terminal->repeat_timer_id = 0;
    }

    terminal->repeat_file_sending = FALSE;

    // Update UI
    gtk_widget_set_sensitive(terminal->send_file_button, TRUE);
    gtk_widget_set_sensitive(terminal->send_file_stop_button, FALSE);

    if (terminal->repeat_filename) {
        char status_msg[512];
        snprintf(status_msg, sizeof(status_msg), "Stopped repeating file %s", terminal->repeat_filename);
        append_to_receive_text(terminal, status_msg, FALSE);
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
