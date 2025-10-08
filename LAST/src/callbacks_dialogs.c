/*
 * Dialog callbacks for LAST - Linux Advanced Serial Transceiver
 * Handles appearance and display options dialog windows
 */

#include "callbacks.h"
#include "ui.h"

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

