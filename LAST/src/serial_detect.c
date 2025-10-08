/*
 * Serial port detection module for LAST - Linux Advanced Serial Transceiver
 * Handles scanning and detection of serial devices
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

