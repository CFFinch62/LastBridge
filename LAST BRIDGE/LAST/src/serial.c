/*
 * Serial communication module for LAST - Linux Advanced Serial Transceiver
 * Handles port detection, connection, configuration, and I/O
 */

#include "serial.h"

void scan_all_serial_devices(GtkComboBoxText *combo) {
    // Clear existing items
    gtk_combo_box_text_remove_all(combo);
    
    // Add manual entry option
    gtk_combo_box_text_append_text(combo, "Custom Path...");
    
    // Scan /dev/ and /tmp/ for all possible serial devices
    const char *prefixes[] = {
        "/dev/ttyS",     // Standard serial ports
        "/dev/ttyUSB",   // USB serial adapters
        "/dev/ttyACM",   // USB CDC ACM devices
        "/dev/ttyV",     // Virtual devices (our null modem)
        "/tmp/ttyV",     // PyVComm virtual devices
        "/dev/pts/",     // PTY devices
        "/dev/rfcomm",   // Bluetooth serial
        NULL
    };
    
    for (int prefix_idx = 0; prefixes[prefix_idx] != NULL; prefix_idx++) {
        const char *prefix = prefixes[prefix_idx];
        
        // For pts, scan directory
        if (strcmp(prefix, "/dev/pts/") == 0) {
            DIR *dir = opendir("/dev/pts");
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_name[0] != '.' && strcmp(entry->d_name, "ptmx") != 0) {
                        char path[256];
                        snprintf(path, sizeof(path), "/dev/pts/%s", entry->d_name);
                        if (is_serial_device(path)) {
                            gtk_combo_box_text_append_text(combo, path);
                        }
                    }
                }
                closedir(dir);
            }
        } else {
            // For other prefixes, try numbers 0-99
            for (int i = 0; i < 100; i++) {
                char path[256];
                snprintf(path, sizeof(path), "%s%d", prefix, i);
                
                if (is_serial_device(path)) {
                    gtk_combo_box_text_append_text(combo, path);
                }
            }
        }
    }
    
    // Scan for any other tty devices in /dev
    DIR *dev_dir = opendir("/dev");
    if (dev_dir) {
        struct dirent *entry;
        while ((entry = readdir(dev_dir)) != NULL) {
            if (strncmp(entry->d_name, "tty", 3) == 0 && strlen(entry->d_name) > 3) {
                char path[256];
                snprintf(path, sizeof(path), "/dev/%s", entry->d_name);
                
                // Skip if already added
                gboolean already_added = FALSE;
                GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
                GtkTreeIter iter;
                if (gtk_tree_model_get_iter_first(model, &iter)) {
                    do {
                        gchar *existing_text;
                        gtk_tree_model_get(model, &iter, 0, &existing_text, -1);
                        if (existing_text && strcmp(existing_text, path) == 0) {
                            already_added = TRUE;
                            g_free(existing_text);
                            break;
                        }
                        g_free(existing_text);
                    } while (gtk_tree_model_iter_next(model, &iter));
                }
                
                if (!already_added && is_serial_device(path)) {
                    gtk_combo_box_text_append_text(combo, path);
                }
            }
        }
        closedir(dev_dir);
    }
}

gboolean is_serial_device(const char *path) {
    struct stat st;
    
    // Check if file exists
    if (stat(path, &st) != 0) {
        return FALSE;
    }
    
    // Check if it's a character device
    if (!S_ISCHR(st.st_mode)) {
        return FALSE;
    }
    
    // Try to open it (this is the real test)
    int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        return FALSE;
    }
    
    // Check if it supports termios (serial interface)
    struct termios tio;
    gboolean is_serial = (tcgetattr(fd, &tio) == 0);
    
    close(fd);
    return is_serial;
}

void connect_serial(SerialTerminal *terminal) {
    const char *port = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->port_combo));
    const char *baudrate_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo));
    const char *databits_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo));
    const char *parity_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo));
    const char *stopbits_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo));
    const char *flow_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo));
    (void)flow_str; // Suppress unused warning - used in apply_serial_settings

    if (!port || !baudrate_str) {
        gtk_label_set_text(GTK_LABEL(terminal->status_label), "Please select port and baud rate");
        return;
    }

    // Handle custom path
    if (strcmp(port, "Custom Path...") == 0) {
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
            port = gtk_entry_get_text(GTK_ENTRY(entry));
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }

    // Open serial port
    terminal->serial_fd = open(port, O_RDWR | O_NOCTTY);
    if (terminal->serial_fd < 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Failed to open %s: %s", port, strerror(errno));
        gtk_label_set_text(GTK_LABEL(terminal->status_label), error_msg);
        return;
    }

    // Apply all serial settings
    apply_serial_settings(terminal);

    // Initialize statistics
    terminal->bytes_sent = 0;
    terminal->bytes_received = 0;
    terminal->connection_start_time = time(NULL);

    // Start read thread
    terminal->connected = TRUE;
    terminal->thread_running = TRUE;
    pthread_create(&terminal->read_thread, NULL, read_thread_func, terminal);

    // Update UI
    gtk_widget_set_sensitive(terminal->connect_button, FALSE);
    gtk_widget_set_sensitive(terminal->disconnect_button, TRUE);
    gtk_widget_set_sensitive(terminal->send_entry, TRUE);
    gtk_widget_set_sensitive(terminal->send_button, TRUE);
    gtk_widget_set_sensitive(terminal->send_file_button, TRUE);
    gtk_widget_set_sensitive(terminal->send_file_repeat_check, TRUE);
    gtk_widget_set_sensitive(terminal->send_file_interval_combo, TRUE);
    gtk_widget_set_sensitive(terminal->break_button, TRUE);

    // Apply control signals
    set_control_signals(terminal);

    char status_msg[256];
    snprintf(status_msg, sizeof(status_msg), "Connected to %s at %s baud (%s%s%s)",
             port, baudrate_str, databits_str, parity_str, stopbits_str);
    gtk_label_set_text(GTK_LABEL(terminal->status_label), status_msg);

    // Log connection
    append_to_receive_text(terminal, status_msg, FALSE);
}

void disconnect_serial(SerialTerminal *terminal) {
    if (!terminal->connected) return;

    terminal->connected = FALSE;
    terminal->thread_running = FALSE;

    // Stop repeat file sending if active
    stop_repeat_file_sending(terminal);

    // Wait for read thread to finish
    pthread_join(terminal->read_thread, NULL);

    // Close log file if open
    if (terminal->log_file) {
        fclose(terminal->log_file);
        terminal->log_file = NULL;
    }

    // Close serial port
    close(terminal->serial_fd);
    terminal->serial_fd = -1;

    // Update UI
    gtk_widget_set_sensitive(terminal->connect_button, TRUE);
    gtk_widget_set_sensitive(terminal->disconnect_button, FALSE);
    gtk_widget_set_sensitive(terminal->send_entry, FALSE);
    gtk_widget_set_sensitive(terminal->send_button, FALSE);
    gtk_widget_set_sensitive(terminal->send_file_button, FALSE);
    gtk_widget_set_sensitive(terminal->send_file_repeat_check, FALSE);
    gtk_widget_set_sensitive(terminal->send_file_interval_combo, FALSE);
    gtk_widget_set_sensitive(terminal->send_file_stop_button, FALSE);
    gtk_widget_set_sensitive(terminal->break_button, FALSE);

    gtk_label_set_text(GTK_LABEL(terminal->status_label), "Disconnected");

    // Log disconnection
    append_to_receive_text(terminal, "Disconnected", FALSE);
}

void *read_thread_func(void *arg) {
    SerialTerminal *terminal = (SerialTerminal *)arg;
    char buffer[1024];

    while (terminal->thread_running) {
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(terminal->serial_fd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100ms timeout

        int result = select(terminal->serial_fd + 1, &readfds, NULL, NULL, &timeout);
        if (result > 0 && FD_ISSET(terminal->serial_fd, &readfds)) {
            ssize_t bytes_read = read(terminal->serial_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';

                // Update statistics
                terminal->bytes_received += bytes_read;

                // Log to file if enabled
                if (terminal->log_file) {
                    char *timestamp = get_current_timestamp();
                    fprintf(terminal->log_file, "[%s] RX: %s", timestamp, buffer);
                    fflush(terminal->log_file);
                    free(timestamp);
                }

                // Format data for display
                char *display_data = format_data_for_display(buffer, terminal->hex_display);
                g_idle_add(append_to_receive_text_idle, display_data);
            }
        }
    }

    return NULL;
}

gboolean append_to_receive_text_idle(gpointer data) {
    char *text = (char *)data;
    if (g_terminal) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_terminal->receive_text));
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);

        // Add timestamp if enabled
        if (g_terminal->show_timestamps) {
            char *timestamp = get_current_timestamp();
            gtk_text_buffer_insert(buffer, &end, timestamp, -1);
            gtk_text_buffer_insert(buffer, &end, " ", -1);
            free(timestamp);
        }

        gtk_text_buffer_insert(buffer, &end, text, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);

        // Auto-scroll to bottom if enabled
        if (g_terminal->autoscroll) {
            GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
            gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(g_terminal->receive_text), mark);
        }
    }
    g_free(text);
    return FALSE;
}

void append_to_receive_text(SerialTerminal *terminal, const char *text, gboolean is_received) {
    (void)terminal; // Suppress unused warning
    (void)is_received; // For future use (different colors, etc.)
    g_idle_add(append_to_receive_text_idle, g_strdup(text));
}

void apply_serial_settings(SerialTerminal *terminal) {
    struct termios tio;
    tcgetattr(terminal->serial_fd, &tio);

    // Get settings from UI
    const char *baudrate_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo));
    const char *databits_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo));
    const char *parity_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo));
    const char *stopbits_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo));
    const char *flow_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo));

    // Set baud rate
    speed_t baud = B9600;  // Default
    if (strcmp(baudrate_str, "300") == 0) baud = B300;
    else if (strcmp(baudrate_str, "1200") == 0) baud = B1200;
    else if (strcmp(baudrate_str, "2400") == 0) baud = B2400;
    else if (strcmp(baudrate_str, "4800") == 0) baud = B4800;
    else if (strcmp(baudrate_str, "9600") == 0) baud = B9600;
    else if (strcmp(baudrate_str, "19200") == 0) baud = B19200;
    else if (strcmp(baudrate_str, "38400") == 0) baud = B38400;
    else if (strcmp(baudrate_str, "57600") == 0) baud = B57600;
    else if (strcmp(baudrate_str, "115200") == 0) baud = B115200;
    else if (strcmp(baudrate_str, "230400") == 0) baud = B230400;
    else if (strcmp(baudrate_str, "460800") == 0) baud = B460800;
    else if (strcmp(baudrate_str, "921600") == 0) baud = B921600;

    cfsetispeed(&tio, baud);
    cfsetospeed(&tio, baud);

    // Set data bits
    tio.c_cflag &= ~CSIZE;
    if (strcmp(databits_str, "5") == 0) tio.c_cflag |= CS5;
    else if (strcmp(databits_str, "6") == 0) tio.c_cflag |= CS6;
    else if (strcmp(databits_str, "7") == 0) tio.c_cflag |= CS7;
    else tio.c_cflag |= CS8;

    // Set parity
    if (strcmp(parity_str, "Even") == 0) {
        tio.c_cflag |= PARENB;
        tio.c_cflag &= ~PARODD;
    } else if (strcmp(parity_str, "Odd") == 0) {
        tio.c_cflag |= PARENB;
        tio.c_cflag |= PARODD;
    } else {
        tio.c_cflag &= ~PARENB;
    }

    // Set stop bits
    if (strcmp(stopbits_str, "2") == 0) {
        tio.c_cflag |= CSTOPB;
    } else {
        tio.c_cflag &= ~CSTOPB;
    }

    // Set flow control
    if (strcmp(flow_str, "Hardware") == 0) {
        tio.c_cflag |= CRTSCTS;
        tio.c_iflag &= ~(IXON | IXOFF | IXANY);
    } else if (strcmp(flow_str, "Software") == 0) {
        tio.c_cflag &= ~CRTSCTS;
        tio.c_iflag |= (IXON | IXOFF | IXANY);
    } else {
        tio.c_cflag &= ~CRTSCTS;
        tio.c_iflag &= ~(IXON | IXOFF | IXANY);
    }

    // Raw mode
    tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tio.c_oflag &= ~OPOST;

    tcsetattr(terminal->serial_fd, TCSANOW, &tio);
}

void send_data(SerialTerminal *terminal) {
    if (!terminal->connected) return;

    const char *text = gtk_entry_get_text(GTK_ENTRY(terminal->send_entry));
    if (strlen(text) == 0) return;

    // Send data with line ending
    ssize_t bytes_written = write(terminal->serial_fd, text, strlen(text));
    if (bytes_written > 0) {
        terminal->bytes_sent += bytes_written;
    }

    // Add line ending if configured
    if (terminal->line_ending && strlen(terminal->line_ending) > 0) {
        bytes_written = write(terminal->serial_fd, terminal->line_ending, strlen(terminal->line_ending));
        if (bytes_written > 0) {
            terminal->bytes_sent += bytes_written;
        }
    }

    // Log to file if enabled
    if (terminal->log_file) {
        char *timestamp = get_current_timestamp();
        fprintf(terminal->log_file, "[%s] TX: %s\n", timestamp, text);
        fflush(terminal->log_file);
        free(timestamp);
    }

    // Local echo if enabled
    if (terminal->local_echo) {
        char echo_text[1024];
        snprintf(echo_text, sizeof(echo_text), "TX: %s", text);
        append_to_receive_text(terminal, echo_text, FALSE);
    }

    // Clear entry
    gtk_entry_set_text(GTK_ENTRY(terminal->send_entry), "");
}

void set_control_signals(SerialTerminal *terminal) {
    if (!terminal->connected) return;

    int status;
    if (ioctl(terminal->serial_fd, TIOCMGET, &status) == -1) {
        return;
    }

    // Set DTR
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->dtr_check))) {
        status |= TIOCM_DTR;
    } else {
        status &= ~TIOCM_DTR;
    }

    // Set RTS
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->rts_check))) {
        status |= TIOCM_RTS;
    } else {
        status &= ~TIOCM_RTS;
    }

    ioctl(terminal->serial_fd, TIOCMSET, &status);
}

void send_break_signal(SerialTerminal *terminal) {
    if (!terminal->connected) return;

    tcsendbreak(terminal->serial_fd, 0);
    append_to_receive_text(terminal, "Break signal sent", FALSE);
}
