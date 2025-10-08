/*
 * Macro and scripting callbacks for LAST - Linux Advanced Serial Transceiver
 * Handles macro button clicks, programming, and scripting window operations
 */

#include "callbacks.h"
#include "serial.h"
#include "network.h"
#include "file_ops.h"
#include "utils.h"
#include "ui.h"
#include "settings.h"
#include "scripting.h"

// Forward declarations for macro chaining
void send_single_command(SerialTerminal *terminal, const char *command, gboolean add_line_ending);
void send_macro_command_parts(SerialTerminal *terminal, const char *command, int macro_index);

void send_single_command(SerialTerminal *terminal, const char *command, gboolean add_line_ending) {
    if (!terminal || !command || !terminal->connected || strlen(command) == 0) return;

    // Send the command based on connection type
    ssize_t bytes_written = 0;

    if (terminal->connection_type == CONNECTION_TYPE_SERIAL) {
        bytes_written = write(terminal->connection_fd, command, strlen(command));
    } else {
        bytes_written = network_send_data(terminal, command, strlen(command));
    }

    if (bytes_written > 0) {
        terminal->bytes_sent += bytes_written;
        // Mark TX activity
        terminal->tx_active = TRUE;
        terminal->tx_last_activity = time(NULL);
    }

    // Add line ending if requested and configured
    if (add_line_ending && terminal->line_ending && strlen(terminal->line_ending) > 0) {
        if (terminal->connection_type == CONNECTION_TYPE_SERIAL) {
            bytes_written = write(terminal->connection_fd, terminal->line_ending, strlen(terminal->line_ending));
        } else {
            bytes_written = network_send_data(terminal, terminal->line_ending, strlen(terminal->line_ending));
        }

        if (bytes_written > 0) {
            terminal->bytes_sent += bytes_written;
            // Mark TX activity for line ending too
            terminal->tx_active = TRUE;
            terminal->tx_last_activity = time(NULL);
        }
    }

    // Log to file if enabled
    if (terminal->log_file) {
        char *timestamp = get_current_timestamp();
        fprintf(terminal->log_file, "[%s] TX: %s\n", timestamp, command);
        fflush(terminal->log_file);
        free(timestamp);
    }

    // Local echo if enabled
    if (terminal->local_echo) {
        char echo_text[1024];
        snprintf(echo_text, sizeof(echo_text), "TX: %s", command);
        append_to_receive_text(terminal, echo_text, FALSE);
    }
}

void send_macro_command_parts(SerialTerminal *terminal, const char *command, int macro_index) {
    if (!terminal || !command || !terminal->connected) return;

    // Parse command into parts to handle macro chaining properly
    MacroParts *parts = parse_macro_command(terminal, command, macro_index);
    if (!parts) {
        // Fallback to sending as single command
        send_single_command(terminal, command, TRUE);
        return;
    }

    // Send each part
    for (int i = 0; i < parts->count; i++) {
        if (parts->parts[i] && strlen(parts->parts[i]) > 0) {
            // Each macro reference gets its own line ending
            // Regular text parts are combined until the next macro reference
            gboolean add_ending = parts->is_macro_ref[i] || (i == parts->count - 1);
            send_single_command(terminal, parts->parts[i], add_ending);
        }
    }

    free_macro_parts(parts);
}

void on_macro_button_clicked(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;

    if (!terminal->connected) return;

    // Get the macro index from the button data
    int macro_index = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "macro_index"));

    if (macro_index < 0 || macro_index >= MAX_MACRO_BUTTONS) return;

    // Get the command to send
    const char *command = terminal->macro_commands[macro_index];

    // Only send if command is not empty
    if (strlen(command) > 0) {
        // Send the command with macro chaining support
        send_macro_command_parts(terminal, command, macro_index);
    }
}

void on_macros_program_activate(GtkWidget *widget, gpointer data) {
    (void)widget; // Suppress unused parameter warning
    SerialTerminal *terminal = (SerialTerminal *)data;
    show_macro_programming_dialog(terminal);
}

void on_macros_toggle_activate(GtkWidget *widget, gpointer data) {
    (void)widget; // Suppress unused parameter warning
    SerialTerminal *terminal = (SerialTerminal *)data;
    toggle_macro_panel_visibility(terminal);
}

// Scripting callbacks
void on_tools_scripting_activate(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    create_scripting_window(terminal);
}

void on_script_window_destroy(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    terminal->script_window = NULL;
}

void on_script_enable_toggled(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    terminal->scripting_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    // Save settings
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_script_load_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Lua Script",
                                                   GTK_WINDOW(terminal->script_window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);

    // Add file filter for Lua files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Lua Scripts (*.lua)");
    gtk_file_filter_add_pattern(filter, "*.lua");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Add filter for all files
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "All Files");
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Load script content
        gchar *script_content = NULL;
        gsize length = 0;
        GError *error = NULL;

        if (g_file_get_contents(filename, &script_content, &length, &error)) {
            // Set script content in text view
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->script_text_view));
            gtk_text_buffer_set_text(buffer, script_content, -1);

            // Load script into engine
            if (terminal->lua_state) {
                scripting_load_script(terminal, script_content);
            }

            g_free(script_content);
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(terminal->script_window),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "Failed to load script file:\n%s",
                                                            error ? error->message : "Unknown error");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            if (error) g_error_free(error);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void on_script_save_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Lua Script",
                                                   GTK_WINDOW(terminal->script_window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    // Add file filter for Lua files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Lua Scripts (*.lua)");
    gtk_file_filter_add_pattern(filter, "*.lua");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Set default filename
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "script.lua");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Get script content from text view
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->script_text_view));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        gchar *script_content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        // Save to file
        GError *error = NULL;
        if (!g_file_set_contents(filename, script_content, -1, &error)) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(terminal->script_window),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "Failed to save script file:\n%s",
                                                            error ? error->message : "Unknown error");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            if (error) g_error_free(error);
        }

        g_free(script_content);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void on_script_test_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    if (!terminal->lua_state) {
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(terminal->script_window),
                                                        GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_OK,
                                                        "Scripting engine not initialized");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return;
    }

    // Get script content from text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->script_text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *script_content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (!script_content || strlen(script_content) == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(terminal->script_window),
                                                        GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_WARNING,
                                                        GTK_BUTTONS_OK,
                                                        "No script content to test");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        g_free(script_content);
        return;
    }

    // Load and test the script
    gboolean success = scripting_load_script(terminal, script_content);

    if (success) {
        GtkWidget *success_dialog = gtk_message_dialog_new(GTK_WINDOW(terminal->script_window),
                                                          GTK_DIALOG_MODAL,
                                                          GTK_MESSAGE_INFO,
                                                          GTK_BUTTONS_OK,
                                                          "Script loaded successfully!\n\n"
                                                          "The script has been compiled and is ready to use.");
        gtk_dialog_run(GTK_DIALOG(success_dialog));
        gtk_widget_destroy(success_dialog);
    } else {
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(terminal->script_window),
                                                        GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_OK,
                                                        "Script compilation failed!\n\n"
                                                        "Please check the script syntax and try again.");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
    }

    g_free(script_content);
}

void on_script_clear_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    // Clear the text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->script_text_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    // Clear the script from the engine
    if (terminal->lua_state) {
        scripting_clear_script(terminal);
    }
}

