/*
 * Connection callbacks for LAST - Linux Advanced Serial Transceiver
 * Handles connection type changes, connect/disconnect, and connection settings
 */

#include "callbacks.h"
#include "serial.h"
#include "network.h"
#include "settings.h"

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
        // Network connection logic
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

