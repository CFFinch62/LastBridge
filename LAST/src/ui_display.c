/*
 * Display area creation for LAST - Linux Advanced Serial Terminal
 * Handles data area, appearance settings, and theme application
 */

#include "ui.h"
#include "callbacks.h"

void create_data_area(SerialTerminal *terminal, GtkWidget *parent) {
    // Receive area - make it less tall
    GtkWidget *receive_frame = gtk_frame_new("Received Data");
    gtk_box_pack_start(GTK_BOX(parent), receive_frame, TRUE, TRUE, 0);

    GtkWidget *receive_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(receive_frame), receive_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(receive_vbox), 5);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    // Enable both vertical and horizontal scroll bars
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    // Set height for receive area - default to half height since hex display is shown by default
    gtk_widget_set_size_request(scrolled, -1, 120); // Half height when hex is shown
    gtk_box_pack_start(GTK_BOX(receive_vbox), scrolled, TRUE, TRUE, 0);

    terminal->receive_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(terminal->receive_text), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(terminal->receive_text), TRUE);
    // Disable text wrapping to enable horizontal scrolling
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(terminal->receive_text), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(scrolled), terminal->receive_text);

    // Create hex display area (shown by default)
    terminal->hex_frame = gtk_frame_new("Hex Data");
    gtk_box_pack_start(GTK_BOX(parent), terminal->hex_frame, TRUE, TRUE, 0);
    // Hex display is shown by default, but can be hidden via toggle

    GtkWidget *hex_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(terminal->hex_frame), hex_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(hex_vbox), 5);

    terminal->hex_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(terminal->hex_scrolled),
                                   GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    // Set height for hex area - will be shown when hex display is enabled
    gtk_widget_set_size_request(terminal->hex_scrolled, -1, 120); // Half the original height
    gtk_box_pack_start(GTK_BOX(hex_vbox), terminal->hex_scrolled, TRUE, TRUE, 0);

    terminal->hex_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(terminal->hex_text), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(terminal->hex_text), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(terminal->hex_text), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(terminal->hex_scrolled), terminal->hex_text);

    // Realize the hex text widget so it can receive styling even when hidden
    gtk_widget_realize(terminal->hex_text);

    // Receive controls
    GtkWidget *receive_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(receive_vbox), receive_controls, FALSE, FALSE, 0);

    terminal->clear_button = gtk_button_new_with_label("Clear");
    gtk_box_pack_start(GTK_BOX(receive_controls), terminal->clear_button, FALSE, FALSE, 0);

    terminal->save_button = gtk_button_new_with_label("Save Received Data...");
    gtk_box_pack_start(GTK_BOX(receive_controls), terminal->save_button, FALSE, FALSE, 0);

    // Add spacer to push indicators to the right
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(receive_controls), spacer, TRUE, TRUE, 0);

    // Create signal line indicators
    create_signal_indicators(terminal, receive_controls);

    // Send area - keep in center
    GtkWidget *send_frame = gtk_frame_new("Send Data");
    gtk_box_pack_start(GTK_BOX(parent), send_frame, FALSE, FALSE, 0);

    GtkWidget *send_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(send_frame), send_hbox);
    gtk_container_set_border_width(GTK_CONTAINER(send_hbox), 5);

    terminal->send_entry = gtk_entry_new();
    gtk_widget_set_sensitive(terminal->send_entry, FALSE);
    gtk_box_pack_start(GTK_BOX(send_hbox), terminal->send_entry, TRUE, TRUE, 0);

    terminal->send_button = gtk_button_new_with_label("Send");
    gtk_widget_set_sensitive(terminal->send_button, FALSE);
    gtk_box_pack_start(GTK_BOX(send_hbox), terminal->send_button, FALSE, FALSE, 0);
}

void apply_appearance_settings(SerialTerminal *terminal) {
    if (!terminal->receive_text) return;

    // Apply font using CSS
    const char *font_desc_str = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(terminal->font_button));
    PangoFontDescription *font_desc = pango_font_description_from_string(font_desc_str ? font_desc_str : "Monospace 10");

    const char *family = pango_font_description_get_family(font_desc);
    int size = pango_font_description_get_size(font_desc) / PANGO_SCALE;

    // Apply colors using CSS
    GdkRGBA bg_color, text_color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(terminal->bg_color_button), &bg_color);
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(terminal->text_color_button), &text_color);

    // Create CSS string with more specific selectors
    char css_data[1024];
    snprintf(css_data, sizeof(css_data),
        "textview, textview text { "
        "font-family: \"%s\"; "
        "font-size: %dpt; "
        "background-color: rgba(%d,%d,%d,%.2f); "
        "color: rgba(%d,%d,%d,%.2f); "
        "} "
        "textview:selected { "
        "background-color: rgba(%d,%d,%d,0.3); "
        "}",
        family ? family : "Monospace",
        size > 0 ? size : 10,
        (int)(bg_color.red * 255), (int)(bg_color.green * 255), (int)(bg_color.blue * 255), bg_color.alpha,
        (int)(text_color.red * 255), (int)(text_color.green * 255), (int)(text_color.blue * 255), text_color.alpha,
        (int)(text_color.red * 255), (int)(text_color.green * 255), (int)(text_color.blue * 255)
    );

    pango_font_description_free(font_desc);

    // Apply CSS with higher priority
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error = NULL;
    gtk_css_provider_load_from_data(css_provider, css_data, -1, &error);

    if (error) {
        g_warning("CSS Error: %s", error->message);
        g_error_free(error);
    }

    // Apply CSS to text display
    GtkStyleContext *text_context = gtk_widget_get_style_context(terminal->receive_text);
    gtk_style_context_add_provider(text_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Apply CSS to hex display if it exists (even if hidden)
    if (terminal->hex_text) {
        GtkStyleContext *hex_context = gtk_widget_get_style_context(terminal->hex_text);
        gtk_style_context_add_provider(hex_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        // Force the hex text view to realize its style even when hidden
        gtk_widget_realize(terminal->hex_text);
    }

    g_object_unref(css_provider);

    // Apply theme
    const char *theme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo));
    if (theme) {
        GtkSettings *settings = gtk_settings_get_default();
        if (strcmp(theme, "Dark") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
            // Force theme update
            g_object_set(settings, "gtk-theme-name", "Adwaita-dark", NULL);
        } else if (strcmp(theme, "Light") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            // Force theme update
            g_object_set(settings, "gtk-theme-name", "Adwaita", NULL);
        } else {
            // System default - reset to system preference
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            g_object_set(settings, "gtk-theme-name", NULL, NULL);
        }

        // Force a style update on all widgets
        GList *toplevels = gtk_window_list_toplevels();
        for (GList *l = toplevels; l != NULL; l = l->next) {
            if (GTK_IS_WIDGET(l->data)) {
                gtk_widget_reset_style(GTK_WIDGET(l->data));
            }
        }
        g_list_free(toplevels);
    }
}

void apply_theme_setting(SerialTerminal *terminal) {
    const char *theme = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->theme_combo));
    if (theme) {
        GtkSettings *settings = gtk_settings_get_default();
        if (strcmp(theme, "Dark") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
            g_object_set(settings, "gtk-theme-name", "Adwaita-dark", NULL);
        } else if (strcmp(theme, "Light") == 0) {
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            g_object_set(settings, "gtk-theme-name", "Adwaita", NULL);
        } else {
            // System default
            g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
            g_object_set(settings, "gtk-theme-name", NULL, NULL);
        }

        // Force immediate style update
        GList *toplevels = gtk_window_list_toplevels();
        for (GList *l = toplevels; l != NULL; l = l->next) {
            if (GTK_IS_WIDGET(l->data)) {
                gtk_widget_reset_style(GTK_WIDGET(l->data));
            }
        }
        g_list_free(toplevels);
    }
}

