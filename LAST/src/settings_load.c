/*
 * Settings loading module for LAST - Linux Advanced Serial Transceiver
 * Handles loading settings from file and applying them to UI
 */

#include "settings.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

char* get_config_file_path(void) {
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    
    char *config_dir = malloc(strlen(home) + 20);
    sprintf(config_dir, "%s/.config", home);
    
    // Create .config directory if it doesn't exist
    mkdir(config_dir, 0755);
    
    char *config_path = malloc(strlen(home) + 40);
    sprintf(config_path, "%s/.config/last.conf", home);
    
    free(config_dir);
    return config_path;
}

void load_settings(SerialTerminal *terminal) {
    char *config_path = get_config_file_path();
    FILE *file = fopen(config_path, "r");
    
    if (!file) {
        free(config_path);
        return; // Use defaults if no config file
    }
    
    char line[512];
    char key[256], value[256];
    
    while (fgets(line, sizeof(line), file)) {
        // Skip comments, empty lines, and section headers
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r' || line[0] == '[') continue;

        if (sscanf(line, "%255[^=]=%255[^\n\r]", key, value) == 2) {
            // Appearance settings
            if (strcmp(key, "theme") == 0) {
                if (terminal->theme_preference) free(terminal->theme_preference);
                terminal->theme_preference = strdup(value);
            } else if (strcmp(key, "font_family") == 0) {
                if (terminal->font_family) free(terminal->font_family);
                terminal->font_family = strdup(value);
            } else if (strcmp(key, "font_size") == 0) {
                terminal->font_size = atoi(value);
            } else if (strcmp(key, "bg_color") == 0) {
                if (terminal->bg_color) free(terminal->bg_color);
                terminal->bg_color = strdup(value);
            } else if (strcmp(key, "text_color") == 0) {
                if (terminal->text_color) free(terminal->text_color);
                terminal->text_color = strdup(value);
            }
            // Display options
            else if (strcmp(key, "hex_display") == 0) {
                terminal->hex_display = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "hex_bytes_per_line") == 0) {
                terminal->hex_bytes_per_line = atoi(value);
            } else if (strcmp(key, "show_timestamps") == 0) {
                terminal->show_timestamps = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "autoscroll") == 0) {
                terminal->autoscroll = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "local_echo") == 0) {
                terminal->local_echo = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "line_ending") == 0) {
                if (terminal->line_ending) free(terminal->line_ending);
                terminal->line_ending = strdup(value);
            }
            // Connection settings
            else if (strcmp(key, "connection_type") == 0) {
                if (terminal->saved_connection_type) free(terminal->saved_connection_type);
                terminal->saved_connection_type = strdup(value);
            } else if (strcmp(key, "port") == 0) {
                if (terminal->saved_port) free(terminal->saved_port);
                terminal->saved_port = strdup(value);
            } else if (strcmp(key, "baudrate") == 0) {
                if (terminal->saved_baudrate) free(terminal->saved_baudrate);
                terminal->saved_baudrate = strdup(value);
            } else if (strcmp(key, "databits") == 0) {
                if (terminal->saved_databits) free(terminal->saved_databits);
                terminal->saved_databits = strdup(value);
            } else if (strcmp(key, "parity") == 0) {
                if (terminal->saved_parity) free(terminal->saved_parity);
                terminal->saved_parity = strdup(value);
            } else if (strcmp(key, "stopbits") == 0) {
                if (terminal->saved_stopbits) free(terminal->saved_stopbits);
                terminal->saved_stopbits = strdup(value);
            } else if (strcmp(key, "flowcontrol") == 0) {
                if (terminal->saved_flowcontrol) free(terminal->saved_flowcontrol);
                terminal->saved_flowcontrol = strdup(value);
            }
            // Network settings
            else if (strcmp(key, "network_host") == 0) {
                if (terminal->saved_network_host) free(terminal->saved_network_host);
                terminal->saved_network_host = strdup(value);
            } else if (strcmp(key, "network_port") == 0) {
                if (terminal->saved_network_port) free(terminal->saved_network_port);
                terminal->saved_network_port = strdup(value);
            }
            // File operations settings
            else if (strcmp(key, "line_by_line_mode") == 0) {
                terminal->line_by_line_mode = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "line_by_line_delay_ms") == 0) {
                terminal->line_by_line_delay_ms = atoi(value);
            }
            // Macro settings
            else if (strcmp(key, "macro_panel_visible") == 0) {
                terminal->macro_panel_visible = (strcmp(value, "true") == 0);
            } else if (strncmp(key, "macro_", 6) == 0) {
                // Parse macro settings (macro_0_label, macro_0_command, etc.)
                char *underscore = strchr(key + 6, '_');
                if (underscore) {
                    int macro_index = atoi(key + 6);

                    if (macro_index >= 0 && macro_index < MAX_MACRO_BUTTONS) {
                        if (strstr(key, "_label")) {
                            strncpy(terminal->macro_labels[macro_index], value, MAX_MACRO_LABEL_LENGTH - 1);
                            terminal->macro_labels[macro_index][MAX_MACRO_LABEL_LENGTH - 1] = '\0';
                        } else if (strstr(key, "_command")) {
                            strncpy(terminal->macro_commands[macro_index], value, MAX_MACRO_COMMAND_LENGTH - 1);
                            terminal->macro_commands[macro_index][MAX_MACRO_COMMAND_LENGTH - 1] = '\0';
                        }
                    }
                }
            }
        }
    }

    fclose(file);
    free(config_path);
}

void apply_loaded_settings(SerialTerminal *terminal) {
    if (!terminal->window) return; // UI not created yet

    // Apply appearance settings
    if (terminal->theme_combo) {
        const char *theme = terminal->theme_preference;
        if (theme) {
            if (strcasecmp(theme, "System") == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->theme_combo), 0);
            } else if (strcasecmp(theme, "Light") == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->theme_combo), 1);
            } else if (strcasecmp(theme, "Dark") == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->theme_combo), 2);
            }
        }
    }

    // Apply font settings
    if (terminal->font_button && terminal->font_family) {
        char font_desc[256];
        snprintf(font_desc, sizeof(font_desc), "%s %d", terminal->font_family, terminal->font_size);
        gtk_font_chooser_set_font(GTK_FONT_CHOOSER(terminal->font_button), font_desc);
    }

    // Apply color settings
    if (terminal->bg_color_button && terminal->bg_color) {
        GdkRGBA color;
        if (gdk_rgba_parse(&color, terminal->bg_color)) {
            gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(terminal->bg_color_button), &color);
        }
    }

    if (terminal->text_color_button && terminal->text_color) {
        GdkRGBA color;
        if (gdk_rgba_parse(&color, terminal->text_color)) {
            gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(terminal->text_color_button), &color);
        }
    }

    // Apply display settings
    if (terminal->hex_display_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->hex_display_check), terminal->hex_display);

        // Apply hex display visibility based on loaded setting
        if (terminal->hex_display && terminal->hex_frame) {
            // Hex display should be shown (default state)
            gtk_widget_show(terminal->hex_frame);
            // Text area height is already set to 120 by default
            gtk_widget_set_size_request(gtk_widget_get_parent(terminal->receive_text), -1, 120);
        } else if (terminal->hex_frame) {
            // Hide hex display if setting is disabled
            gtk_widget_hide(terminal->hex_frame);
            // Restore text area to full height
            gtk_widget_set_size_request(gtk_widget_get_parent(terminal->receive_text), -1, 240);
        }
    }
    if (terminal->hex_bytes_per_line_combo) {
        // Set the combo box based on the saved value
        if (terminal->hex_bytes_per_line == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo), 0); // Auto (CR+LF)
        } else if (terminal->hex_bytes_per_line == 8) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo), 1);
        } else if (terminal->hex_bytes_per_line == 16) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo), 2);
        } else if (terminal->hex_bytes_per_line == 32) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo), 3);
        } else if (terminal->hex_bytes_per_line == 64) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo), 4);
        } else {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->hex_bytes_per_line_combo), 0); // Default to Auto
        }
    }
    if (terminal->timestamp_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->timestamp_check), terminal->show_timestamps);
    }
    if (terminal->autoscroll_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->autoscroll_check), terminal->autoscroll);
    }
    if (terminal->local_echo_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->local_echo_check), terminal->local_echo);
    }

    // Apply line ending setting
    if (terminal->line_ending_combo && terminal->line_ending) {
        if (strcmp(terminal->line_ending, "") == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->line_ending_combo), 0); // None
        } else if (strcmp(terminal->line_ending, "\r") == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->line_ending_combo), 1); // CR
        } else if (strcmp(terminal->line_ending, "\n") == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->line_ending_combo), 2); // LF
        } else if (strcmp(terminal->line_ending, "\r\n") == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->line_ending_combo), 3); // CR+LF
        }
    }

    // Apply connection type setting
    if (terminal->saved_connection_type && terminal->connection_type_combo) {
        int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->connection_type_combo)), NULL);
        for (int i = 0; i < count; i++) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->connection_type_combo), i);
            const char *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->connection_type_combo));
            if (text && strcmp(text, terminal->saved_connection_type) == 0) {
                break;
            }
        }
    }

    // Apply network settings
    if (terminal->saved_network_host && terminal->network_host_entry) {
        gtk_entry_set_text(GTK_ENTRY(terminal->network_host_entry), terminal->saved_network_host);
    }
    if (terminal->saved_network_port && terminal->network_port_entry) {
        gtk_entry_set_text(GTK_ENTRY(terminal->network_port_entry), terminal->saved_network_port);
    }

    // Apply connection settings
    if (terminal->saved_baudrate && terminal->baudrate_combo) {
        // Find and set the saved baudrate
        int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->baudrate_combo)), NULL);
        for (int i = 0; i < count; i++) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->baudrate_combo), i);
            const char *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo));
            if (text && strcmp(text, terminal->saved_baudrate) == 0) {
                break;
            }
        }
    }
    if (terminal->saved_databits && terminal->databits_combo) {
        int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->databits_combo)), NULL);
        for (int i = 0; i < count; i++) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->databits_combo), i);
            const char *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->databits_combo));
            if (text && strcmp(text, terminal->saved_databits) == 0) {
                break;
            }
        }
    }
    if (terminal->saved_parity && terminal->parity_combo) {
        int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->parity_combo)), NULL);
        for (int i = 0; i < count; i++) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->parity_combo), i);
            const char *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->parity_combo));
            if (text && strcmp(text, terminal->saved_parity) == 0) {
                break;
            }
        }
    }
    if (terminal->saved_stopbits && terminal->stopbits_combo) {
        int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->stopbits_combo)), NULL);
        for (int i = 0; i < count; i++) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->stopbits_combo), i);
            const char *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->stopbits_combo));
            if (text && strcmp(text, terminal->saved_stopbits) == 0) {
                break;
            }
        }
    }
    if (terminal->saved_flowcontrol && terminal->flowcontrol_combo) {
        int count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(GTK_COMBO_BOX(terminal->flowcontrol_combo)), NULL);
        for (int i = 0; i < count; i++) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->flowcontrol_combo), i);
            const char *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->flowcontrol_combo));
            if (text && strcmp(text, terminal->saved_flowcontrol) == 0) {
                break;
            }
        }
    }
}

