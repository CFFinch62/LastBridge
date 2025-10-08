/*
 * Menu callbacks for LAST - Linux Advanced Serial Transceiver
 * Handles menu item actions and application lifecycle
 */

#include "callbacks.h"
#include "serial.h"
#include "settings.h"
#include "scripting.h"

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

