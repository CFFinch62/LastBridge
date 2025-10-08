/*
 * Event handlers module for LAST - Linux Advanced Serial Transceiver
 * Contains all GTK callback functions
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

// Connection callbacks
void on_connection_type_changed(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    const char *selection = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

    if (!selection) return;

    // Update connection type
    terminal->connection_type = string_to_connection_type(selection);

    // Show/hide appropriate settings frames
    if (terminal->connection_type == CONNECTION_TYPE_SERIAL) {
        gtk_widget_show_all(terminal->serial_settings_frame);
        gtk_widget_hide(terminal->network_settings_frame);
    } else {
        gtk_widget_hide(terminal->serial_settings_frame);
        gtk_widget_set_no_show_all(terminal->network_settings_frame, FALSE);
        gtk_widget_show_all(terminal->network_settings_frame);
    }

    // Update settings for persistence
    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_connect_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    if (terminal->connection_type == CONNECTION_TYPE_SERIAL) {
        connect_serial(terminal);
    } else {
        // Network connection logic will be added here
        const char *host = gtk_entry_get_text(GTK_ENTRY(terminal->network_host_entry));
        const char *port_str = gtk_entry_get_text(GTK_ENTRY(terminal->network_port_entry));

        if (!is_valid_hostname(host)) {
            show_network_status(terminal, "Invalid hostname or IP address");
            return;
        }

        if (!is_valid_port(port_str)) {
            show_network_status(terminal, "Invalid port number (1-65535)");
            return;
        }

        int port = atoi(port_str);
        gboolean success = FALSE;

        // Store network settings
        strncpy(terminal->network_host, host, MAX_HOSTNAME_LENGTH - 1);
        terminal->network_host[MAX_HOSTNAME_LENGTH - 1] = '\0';
        strncpy(terminal->network_port, port_str, MAX_PORT_LENGTH - 1);
        terminal->network_port[MAX_PORT_LENGTH - 1] = '\0';

        // Initialize connection fields
        terminal->connection_fd = -1;
        terminal->server_fd = -1;

        switch (terminal->connection_type) {
            case CONNECTION_TYPE_TCP_CLIENT:
                success = connect_tcp_client(terminal, host, port);
                break;
            case CONNECTION_TYPE_TCP_SERVER:
                success = connect_tcp_server(terminal, port);
                break;
            case CONNECTION_TYPE_UDP_CLIENT:
                success = connect_udp_client(terminal, host, port);
                break;
            case CONNECTION_TYPE_UDP_SERVER:
                success = connect_udp_server(terminal, port);
                break;
            default:
                success = FALSE;
                break;
        }

        if (success) {
            // Initialize statistics
            terminal->bytes_sent = 0;
            terminal->bytes_received = 0;
            terminal->connection_start_time = time(NULL);

            // Start read thread
            terminal->connected = TRUE;
            terminal->thread_running = TRUE;
            pthread_create(&terminal->read_thread, NULL, network_read_thread_func, terminal);

            // Update UI
            gtk_widget_set_sensitive(terminal->connect_button, FALSE);
            gtk_widget_set_sensitive(terminal->disconnect_button, TRUE);

            // Show connection info
            char *info = get_network_connection_info(terminal);
            if (info) {
                show_network_status(terminal, info);
                free(info);
            }
        }
    }
}

void on_disconnect_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    if (terminal->connection_type == CONNECTION_TYPE_SERIAL) {
        disconnect_serial(terminal);
    } else {
        // Network disconnect
        if (terminal->connected) {
            terminal->thread_running = FALSE;

            if (terminal->read_thread) {
                pthread_join(terminal->read_thread, NULL);
            }

            disconnect_network(terminal);
            terminal->connected = FALSE;

            // Update UI
            gtk_widget_set_sensitive(terminal->connect_button, TRUE);
            gtk_widget_set_sensitive(terminal->disconnect_button, FALSE);

            show_network_status(terminal, "Disconnected");
        }
    }
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

void on_send_file_lines_toggled(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;
    update_interval_dropdown_based_on_mode(terminal);
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

    // Show/hide hex display area and adjust text area size
    if (terminal->hex_display && terminal->hex_frame) {
        // Show hex display
        gtk_widget_show(terminal->hex_frame);
        // Reduce text area height to make room for hex display
        gtk_widget_set_size_request(gtk_widget_get_parent(terminal->receive_text), -1, 120);
    } else if (terminal->hex_frame) {
        // Hide hex display
        gtk_widget_hide(terminal->hex_frame);
        // Restore text area to full height
        gtk_widget_set_size_request(gtk_widget_get_parent(terminal->receive_text), -1, 240);
    }

    update_settings_from_ui(terminal);
    save_settings(terminal);
}

void on_hex_bytes_per_line_changed(GtkWidget *widget, gpointer data) {
    SerialTerminal *terminal = (SerialTerminal *)data;
    const char *selection = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

    if (strcmp(selection, "Auto (CR+LF)") == 0) {
        terminal->hex_bytes_per_line = 0;  // 0 means auto mode (CR+LF detection)
    } else {
        terminal->hex_bytes_per_line = atoi(selection);  // Convert string to int
    }

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

    // Clean up scripting engine
    scripting_cleanup(terminal);

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
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), "1.1");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
        "Linux Advanced Serial Transceiver\n\n"
        "A comprehensive serial communication terminal with advanced features including:\n"
        "• Complete serial parameter control\n"
        "• Hex display mode and timestamps\n"
        "• File operations and data logging\n"
        "• Control signals (DTR, RTS, Break)\n"
        "• Lua scripting engine with dedicated window\n"
        "• Programmable macros and network connectivity\n"
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

// View menu callbacks
void on_view_appearance_activate(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    // Create appearance settings dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Appearance Settings",
        GTK_WINDOW(terminal->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_OK", GTK_RESPONSE_OK,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Theme selection
    GtkWidget *theme_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), theme_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(theme_hbox), gtk_label_new("Theme:"), FALSE, FALSE, 0);

    GtkWidget *theme_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo), "System");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo), "Light");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo), "Dark");

    // Set current theme selection
    const char *current_theme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo));
    if (current_theme) {
        if (strcmp(current_theme, "System") == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(theme_combo), 0);
        else if (strcmp(current_theme, "Light") == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(theme_combo), 1);
        else if (strcmp(current_theme, "Dark") == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(theme_combo), 2);
        else gtk_combo_box_set_active(GTK_COMBO_BOX(theme_combo), 0); // Default to System
        g_free((gchar*)current_theme);
    }

    gtk_box_pack_start(GTK_BOX(theme_hbox), theme_combo, TRUE, TRUE, 0);

    // Font selection
    GtkWidget *font_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), font_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(font_hbox), gtk_label_new("Font:"), FALSE, FALSE, 0);

    GtkWidget *font_button = gtk_font_button_new();
    const char *current_font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(terminal->font_button));
    if (current_font) {
        gtk_font_button_set_font_name(GTK_FONT_BUTTON(font_button), current_font);
    }
    gtk_box_pack_start(GTK_BOX(font_hbox), font_button, TRUE, TRUE, 0);

    // Background color
    GtkWidget *bg_color_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), bg_color_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bg_color_hbox), gtk_label_new("Background Color:"), FALSE, FALSE, 0);

    GtkWidget *bg_color_button = gtk_color_button_new();
    GdkRGBA bg_color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(terminal->bg_color_button), &bg_color);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(bg_color_button), &bg_color);
    gtk_box_pack_start(GTK_BOX(bg_color_hbox), bg_color_button, FALSE, FALSE, 0);

    // Text color
    GtkWidget *text_color_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), text_color_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(text_color_hbox), gtk_label_new("Text Color:"), FALSE, FALSE, 0);

    GtkWidget *text_color_button = gtk_color_button_new();
    GdkRGBA text_color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(terminal->text_color_button), &text_color);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(text_color_button), &text_color);
    gtk_box_pack_start(GTK_BOX(text_color_hbox), text_color_button, FALSE, FALSE, 0);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        // Apply changes to the actual widgets
        const char *selected_theme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(theme_combo));
        if (selected_theme) {
            // Find and set the theme in the original combo
            GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->theme_combo));
            GtkTreeIter iter;
            gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
            int index = 0;
            while (valid) {
                gchar *text;
                gtk_tree_model_get(model, &iter, 0, &text, -1);
                if (strcmp(text, selected_theme) == 0) {
                    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->theme_combo), index);
                    g_free(text);
                    break;
                }
                g_free(text);
                valid = gtk_tree_model_iter_next(model, &iter);
                index++;
            }
            g_free((gchar*)selected_theme);
        }

        // Apply font
        const char *selected_font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_button));
        if (selected_font) {
            gtk_font_button_set_font_name(GTK_FONT_BUTTON(terminal->font_button), selected_font);
        }

        // Apply colors
        GdkRGBA new_bg_color, new_text_color;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(bg_color_button), &new_bg_color);
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(text_color_button), &new_text_color);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(terminal->bg_color_button), &new_bg_color);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(terminal->text_color_button), &new_text_color);

        // Trigger the callbacks to apply changes
        on_theme_changed(terminal->theme_combo, terminal);
        on_font_changed(terminal->font_button, terminal);
        on_bg_color_changed(terminal->bg_color_button, terminal);
        on_text_color_changed(terminal->text_color_button, terminal);
    }

    gtk_widget_destroy(dialog);
}

void on_view_display_options_activate(GtkWidget *widget, gpointer data) {
    (void)widget;
    SerialTerminal *terminal = (SerialTerminal *)data;

    // Create display options dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Display Options",
        GTK_WINDOW(terminal->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_OK", GTK_RESPONSE_OK,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Hex display checkbox
    GtkWidget *hex_display_check = gtk_check_button_new_with_label("Hex Display");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hex_display_check),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->hex_display_check)));
    gtk_box_pack_start(GTK_BOX(vbox), hex_display_check, FALSE, FALSE, 0);

    // Hex bytes per line
    GtkWidget *hex_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hex_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hex_hbox), gtk_label_new("Hex bytes per line:"), FALSE, FALSE, 0);

    GtkWidget *hex_bytes_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "Auto (CR+LF)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "8");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "16");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "32");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "64");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "128");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "256");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(hex_bytes_combo), "512");

    // Set current selection
    int current_hex_selection = gtk_combo_box_get_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo));
    gtk_combo_box_set_active(GTK_COMBO_BOX(hex_bytes_combo), current_hex_selection);
    gtk_box_pack_start(GTK_BOX(hex_hbox), hex_bytes_combo, TRUE, TRUE, 0);

    // Timestamp checkbox
    GtkWidget *timestamp_check = gtk_check_button_new_with_label("Show Timestamps");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timestamp_check),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->timestamp_check)));
    gtk_box_pack_start(GTK_BOX(vbox), timestamp_check, FALSE, FALSE, 0);

    // Autoscroll checkbox
    GtkWidget *autoscroll_check = gtk_check_button_new_with_label("Auto Scroll");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoscroll_check),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->autoscroll_check)));
    gtk_box_pack_start(GTK_BOX(vbox), autoscroll_check, FALSE, FALSE, 0);

    // Local echo checkbox
    GtkWidget *local_echo_check = gtk_check_button_new_with_label("Local Echo");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(local_echo_check),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->local_echo_check)));
    gtk_box_pack_start(GTK_BOX(vbox), local_echo_check, FALSE, FALSE, 0);

    // Line ending selection
    GtkWidget *line_ending_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), line_ending_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(line_ending_hbox), gtk_label_new("Line Ending:"), FALSE, FALSE, 0);

    GtkWidget *line_ending_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(line_ending_combo), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(line_ending_combo), "CR");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(line_ending_combo), "LF");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(line_ending_combo), "CR+LF");

    // Set current line ending selection
    int current_line_ending = gtk_combo_box_get_active(GTK_COMBO_BOX(terminal->line_ending_combo));
    gtk_combo_box_set_active(GTK_COMBO_BOX(line_ending_combo), current_line_ending);
    gtk_box_pack_start(GTK_BOX(line_ending_hbox), line_ending_combo, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        // Apply changes to the actual widgets
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->hex_display_check),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hex_display_check)));

        gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo),
            gtk_combo_box_get_active(GTK_COMBO_BOX(hex_bytes_combo)));

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->timestamp_check),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(timestamp_check)));

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->autoscroll_check),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoscroll_check)));

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->local_echo_check),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(local_echo_check)));

        gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->line_ending_combo),
            gtk_combo_box_get_active(GTK_COMBO_BOX(line_ending_combo)));

        // Trigger the callbacks to apply changes
        on_hex_display_toggled(terminal->hex_display_check, terminal);
        on_hex_bytes_per_line_changed(terminal->hex_bytes_per_line_combo, terminal);
        on_timestamp_toggled(terminal->timestamp_check, terminal);
        on_autoscroll_toggled(terminal->autoscroll_check, terminal);
        on_local_echo_toggled(terminal->local_echo_check, terminal);
        on_line_ending_changed(terminal->line_ending_combo, terminal);
    }

    gtk_widget_destroy(dialog);
}

// Function to connect all signals
void connect_signals(SerialTerminal *terminal) {
    // Connection signals
    g_signal_connect(terminal->connection_type_combo, "changed", G_CALLBACK(on_connection_type_changed), terminal);
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
    g_signal_connect(terminal->send_file_lines_check, "toggled", G_CALLBACK(on_send_file_lines_toggled), terminal);
    g_signal_connect(terminal->log_file_button, "toggled", G_CALLBACK(on_log_toggled), terminal);

    // Control signal signals
    g_signal_connect(terminal->dtr_check, "toggled", G_CALLBACK(on_dtr_toggled), terminal);
    g_signal_connect(terminal->rts_check, "toggled", G_CALLBACK(on_rts_toggled), terminal);
    g_signal_connect(terminal->break_button, "clicked", G_CALLBACK(on_break_clicked), terminal);

    // Display option signals
    g_signal_connect(terminal->hex_display_check, "toggled", G_CALLBACK(on_hex_display_toggled), terminal);
    g_signal_connect(terminal->hex_bytes_per_line_combo, "changed", G_CALLBACK(on_hex_bytes_per_line_changed), terminal);
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
