/*
 * Display and appearance callbacks for LAST - Linux Advanced Serial Transceiver
 * Handles display options and appearance settings
 */

#include "callbacks.h"
#include "settings.h"
#include "ui.h"

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

