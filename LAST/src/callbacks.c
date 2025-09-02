/*
 * Event handlers module for LAST - Linux Advanced Serial Transceiver
 * Contains all GTK callback functions
 */

#include "callbacks.h"
#include "serial.h"
#include "file_ops.h"
#include "utils.h"
#include "ui.h"
#include "settings.h"

// Connection callbacks
void on_connect_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    connect_serial((SerialTerminal *)data);
}

void on_disconnect_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    disconnect_serial((SerialTerminal *)data);
}

void on_refresh_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    scan_all_serial_devices(GTK_COMBO_BOX_TEXT(terminal->port_combo));
}

void on_connection_setting_changed(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;

    // Check if this is the port combo and "Custom Path..." was selected
    if (widget == terminal->port_combo) {
        const char *selected_port = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->port_combo));
        if (selected_port && strcmp(selected_port, "Custom Path...") == 0) {
            // Show custom path dialog
            GtkWidget *dialog = gtk_dialog_new_with_buttons("Enter Custom Port Path",
                GTK_WINDOW(terminal->window),
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                "_OK", GTK_RESPONSE_OK,
                "_Cancel", GTK_RESPONSE_CANCEL,
                NULL);

            GtkWidget *entry = gtk_entry_new();
            gtk_entry_set_text(GTK_ENTRY(entry), "/dev/ttyV0");
            gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), entry);
            gtk_widget_show_all(dialog);

            if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
                const char *custom_path = gtk_entry_get_text(GTK_ENTRY(entry));
                // Add the custom path to the combo box and select it
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->port_combo), custom_path);
                // Find and select the newly added item
                GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->port_combo));
                GtkTreeIter iter;
                gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
                int index = 0;
                while (valid) {
                    gchar *text;
                    gtk_tree_model_get(model, &iter, 0, &text, -1);
                    if (strcmp(text, custom_path) == 0) {
                        gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->port_combo), index);
                        g_free(text);
                        break;
                    }
                    g_free(text);
                    valid = gtk_tree_model_iter_next(model, &iter);
                    index++;
                }
            } else {
                // User cancelled, revert to first item (or previous selection)
                gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->port_combo), 1); // Skip "Custom Path..." and select first real port
            }
            gtk_widget_destroy(dialog);
            g_free((gchar*)selected_port);
            return; // Don't call update_settings_from_ui for "Custom Path..." selection
        }
        g_free((gchar*)selected_port);
    }

    update_settings_from_ui(terminal);
    save_settings(terminal);
}

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

// Display option callbacks
void on_hex_display_toggled(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    terminal->hex_display = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_timestamp_toggled(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    terminal->show_timestamps = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_autoscroll_toggled(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    terminal->autoscroll = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_local_echo_toggled(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    terminal->local_echo = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_line_ending_changed(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    const char *selection = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

    if (terminal->line_ending) free(terminal->line_ending);

    if (strcmp(selection, "CR") == 0) {
        terminal->line_ending = strdup("\r");
    } else if (strcmp(selection, "LF") == 0) {
        terminal->line_ending = strdup("\n");
    } else if (strcmp(selection, "CR+LF") == 0) {
        terminal->line_ending = strdup("\r\n");
    } else {
        terminal->line_ending = strdup("");
    }

    update_settings_from_ui(terminal);
    save_settings(terminal);
}

// Appearance callbacks
void on_theme_changed(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    apply_theme_setting(terminal);
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_font_changed(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    apply_appearance_settings(terminal);
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_bg_color_changed(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    apply_appearance_settings(terminal);
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_text_color_changed(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    apply_appearance_settings(terminal);
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

// Application lifecycle callbacks
void on_window_destroy(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    // Save settings before exit
    update_settings_from_ui(terminal);
    save_settings(terminal);

    // Clean up before exit
    if (terminal->connected) {
        disconnect_serial(terminal);
    }

    // Free allocated memory
    if (terminal->line_ending) free(terminal->line_ending);
    if (terminal->log_filename) free(terminal->log_filename);
    if (terminal->repeat_filename) free(terminal->repeat_filename);
    if (terminal->font_family) free(terminal->font_family);
    if (terminal->bg_color) free(terminal->bg_color);
    if (terminal->text_color) free(terminal->text_color);
    if (terminal->theme_preference) free(terminal->theme_preference);
    if (terminal->saved_port) free(terminal->saved_port);
    if (terminal->saved_baudrate) free(terminal->saved_baudrate);
    if (terminal->saved_databits) free(terminal->saved_databits);
    if (terminal->saved_parity) free(terminal->saved_parity);
    if (terminal->saved_stopbits) free(terminal->saved_stopbits);
    if (terminal->saved_flowcontrol) free(terminal->saved_flowcontrol);

    gtk_main_quit();
}

// Menu callbacks
void on_file_exit_activate(GtkWidget *widget, gpointer data) {
    (void)widget;

    // Call the same cleanup as window destroy
    on_window_destroy(NULL, data);
}

void on_tools_bridge_activate(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;

    // Launch BRIDGE application
    GError *error = NULL;
    gboolean launched = FALSE;

    // Strategy 1: Try to launch BRIDGE from system PATH (for installed version)
    // This will work when both LAST and BRIDGE are installed to /usr/local/bin/
    if (g_spawn_async(NULL, // working directory (NULL = current)
                      (char*[]){"bridge", NULL}, // argv - just "bridge" to use PATH
                      NULL, // envp
                      G_SPAWN_SEARCH_PATH, // flags
                      NULL, // child_setup
                      NULL, // user_data
                      NULL, // child_pid
                      &error)) {
        launched = TRUE;
    } else {
        // Strategy 2: Try relative path (for development environment)
        g_clear_error(&error);
        char *bridge_path = "../BRIDGE/bridge";

        if (g_spawn_async(NULL,
                          (char*[]){bridge_path, NULL},
                          NULL,
                          G_SPAWN_SEARCH_PATH,
                          NULL,
                          NULL,
                          NULL,
                          &error)) {
            launched = TRUE;
        } else {
            // Strategy 3: Try absolute path from current directory
            g_clear_error(&error);
            bridge_path = g_build_filename(g_get_current_dir(), "..", "BRIDGE", "bridge", NULL);

            if (g_spawn_async(NULL,
                              (char*[]){bridge_path, NULL},
                              NULL,
                              G_SPAWN_SEARCH_PATH,
                              NULL,
                              NULL,
                              NULL,
                              &error)) {
                launched = TRUE;
            }
            g_free(bridge_path);
        }
    }

    // If all strategies failed, show error dialog
    if (!launched) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(((SerialTerminal*)data)->window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Failed to launch BRIDGE application.\n\n"
                                                  "Please ensure BRIDGE is installed or available in:\n"
                                                  "• System PATH (if installed)\n"
                                                  "• ../BRIDGE/bridge (development)\n\n"
                                                  "Error: %s",
                                                  error ? error->message : "Unknown error");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (error) {
            g_error_free(error);
        }
    }
}

void on_help_about_activate(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    GtkWidget *about_dialog = gtk_about_dialog_new();

    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "LAST");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), "1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
        "Linux Advanced Serial Transceiver\n\n"
        "A comprehensive serial communication terminal with advanced features including:\n"
        "• Complete serial parameter control\n"
        "• Hex display mode and timestamps\n"
        "• File operations and data logging\n"
        "• Control signals (DTR, RTS, Break)\n"
        "• Professional GUI interface\n"
        "• Integration with BRIDGE virtual null modem");

    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog),
        "Copyright © 2025 Fragillidae Software");

    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog),
        (const char*[]){"Chuck Finch", NULL});

    gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about_dialog),
        "MIT License\n\n"
        "Permission is hereby granted, free of charge, to any person obtaining a copy "
        "of this software and associated documentation files (the \"Software\"), to deal "
        "in the Software without restriction, including without limitation the rights "
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
        "copies of the Software, and to permit persons to whom the Software is "
        "furnished to do so, subject to the following conditions:\n\n"
        "The above copyright notice and this permission notice shall be included in all "
        "copies or substantial portions of the Software.\n\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
        "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
        "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
        "SOFTWARE.");

    gtk_about_dialog_set_wrap_license(GTK_ABOUT_DIALOG(about_dialog), TRUE);
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog), "https://github.com/fragillidae-software");
    gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog), "Fragillidae Software");

    gtk_window_set_transient_for(GTK_WINDOW(about_dialog), GTK_WINDOW(terminal->window));
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);
}

// Function to connect all signals
void connect_signals(SerialTerminal *terminal) {
    // Connection signals
    g_signal_connect(terminal->connect_button, "clicked", G_CALLBACK(on_connect_clicked), terminal);
    g_signal_connect(terminal->disconnect_button, "clicked", G_CALLBACK(on_disconnect_clicked), terminal);
    g_signal_connect(terminal->refresh_button, "clicked", G_CALLBACK(on_refresh_clicked), terminal);

    // Connection setting signals
    g_signal_connect(terminal->port_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->baudrate_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->databits_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->parity_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->stopbits_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);
    g_signal_connect(terminal->flowcontrol_combo, "changed", G_CALLBACK(on_connection_setting_changed), terminal);

    // Data transmission signals
    g_signal_connect(terminal->send_entry, "activate", G_CALLBACK(on_send_activate), terminal);
    g_signal_connect(terminal->send_button, "clicked", G_CALLBACK(on_send_clicked), terminal);

    // Data management signals
    g_signal_connect(terminal->clear_button, "clicked", G_CALLBACK(on_clear_clicked), terminal);
    g_signal_connect(terminal->save_button, "clicked", G_CALLBACK(on_save_clicked), terminal);

    // File operation signals
    g_signal_connect(terminal->send_file_button, "clicked", G_CALLBACK(on_send_file_clicked), terminal);
    g_signal_connect(terminal->send_file_stop_button, "clicked", G_CALLBACK(on_send_file_stop_clicked), terminal);
    g_signal_connect(terminal->log_file_button, "toggled", G_CALLBACK(on_log_toggled), terminal);

    // Control signal signals
    g_signal_connect(terminal->dtr_check, "toggled", G_CALLBACK(on_dtr_toggled), terminal);
    g_signal_connect(terminal->rts_check, "toggled", G_CALLBACK(on_rts_toggled), terminal);
    g_signal_connect(terminal->break_button, "clicked", G_CALLBACK(on_break_clicked), terminal);

    // Display option signals
    g_signal_connect(terminal->hex_display_check, "toggled", G_CALLBACK(on_hex_display_toggled), terminal);
    g_signal_connect(terminal->timestamp_check, "toggled", G_CALLBACK(on_timestamp_toggled), terminal);
    g_signal_connect(terminal->autoscroll_check, "toggled", G_CALLBACK(on_autoscroll_toggled), terminal);
    g_signal_connect(terminal->local_echo_check, "toggled", G_CALLBACK(on_local_echo_toggled), terminal);
    g_signal_connect(terminal->line_ending_combo, "changed", G_CALLBACK(on_line_ending_changed), terminal);

    // Appearance signals
    g_signal_connect(terminal->theme_combo, "changed", G_CALLBACK(on_theme_changed), terminal);
    g_signal_connect(terminal->font_button, "font-set", G_CALLBACK(on_font_changed), terminal);
    g_signal_connect(terminal->bg_color_button, "color-set", G_CALLBACK(on_bg_color_changed), terminal);
    g_signal_connect(terminal->text_color_button, "color-set", G_CALLBACK(on_text_color_changed), terminal);

    // Application lifecycle signals
    g_signal_connect(terminal->window, "destroy", G_CALLBACK(on_window_destroy), terminal);
}
