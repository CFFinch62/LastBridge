/*
 * Main interface creation for LAST - Linux Advanced Serial Terminal
 * Handles main window creation and layout
 */

#include "ui.h"
#include "callbacks.h"
#include "settings.h"

void create_main_interface(SerialTerminal *terminal) {
    // Create main window
    terminal->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(terminal->window), "LAST - Linux Advanced Serial Terminal");

    // Set default size to fit 1366x768 screens with room for decorations and taskbar
    gtk_window_set_default_size(GTK_WINDOW(terminal->window), 1200, 720);

    // Make window fully resizable with maximize button enabled
    gtk_window_set_resizable(GTK_WINDOW(terminal->window), TRUE);

    // Set normal window type hint for proper window manager treatment
    gtk_window_set_type_hint(GTK_WINDOW(terminal->window), GDK_WINDOW_TYPE_HINT_NORMAL);

    // Set minimum size to ensure UI remains usable when resized
    // This allows maximize button while preventing window from being too small
    GdkGeometry geometry;
    geometry.min_width = 800;   // Minimum width for usable UI
    geometry.min_height = 500;  // Minimum height for usable UI
    gtk_window_set_geometry_hints(GTK_WINDOW(terminal->window), NULL, &geometry,
                                  GDK_HINT_MIN_SIZE);

    // Center the window on screen
    gtk_window_set_position(GTK_WINDOW(terminal->window), GTK_WIN_POS_CENTER);

    // Set window icon - try multiple approaches
    GdkPixbuf *icon = NULL;
    GError *error = NULL;

    // Try loading PNG icon first (smaller, better for window icons)
    const char *icon_paths[] = {
        "last-icon.png",                           // Current directory (PNG)
        "last-icon.jpg",                           // Current directory (JPG fallback)
        "/usr/local/share/pixmaps/last-icon.png",  // System location (PNG)
        "/usr/local/share/pixmaps/last-icon.jpg",  // System location (JPG fallback)
        "/usr/share/pixmaps/last-icon.png",        // Alternative system location (PNG)
        "/usr/share/pixmaps/last-icon.jpg",        // Alternative system location (JPG fallback)
        NULL
    };

    for (int i = 0; icon_paths[i] != NULL && !icon; i++) {
        error = NULL;
        icon = gdk_pixbuf_new_from_file(icon_paths[i], &error);
        if (icon) {
            // Scale icon to appropriate size for window decoration
            GdkPixbuf *scaled_icon = gdk_pixbuf_scale_simple(icon, 48, 48, GDK_INTERP_BILINEAR);
            if (scaled_icon) {
                gtk_window_set_icon(GTK_WINDOW(terminal->window), scaled_icon);
                g_object_unref(scaled_icon);
            }
            g_object_unref(icon);
            break;
        } else if (error) {
            g_error_free(error);
        }
    }

    // Create main container
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(terminal->window), main_vbox);

    // Create menu bar
    create_menu_bar(terminal, main_vbox);

    // Create main horizontal layout
    terminal->main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_box_pack_start(GTK_BOX(main_vbox), terminal->main_hbox, TRUE, TRUE, 2);

    // Create scrolled window for left panel
    GtkWidget *left_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_scrolled),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(left_scrolled, 300, -1);
    gtk_box_pack_start(GTK_BOX(terminal->main_hbox), left_scrolled, FALSE, FALSE, 15);

    // Create left panel
    GtkWidget *left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_add(GTK_CONTAINER(left_scrolled), left_panel);

    create_connection_panel(terminal, left_panel);
    create_control_signals_panel(terminal, left_panel);
    create_file_operations_panel(terminal, left_panel);

    // Create macro panel
    terminal->macro_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_widget_set_size_request(terminal->macro_panel, 200, -1);
    gtk_box_pack_start(GTK_BOX(terminal->main_hbox), terminal->macro_panel, FALSE, FALSE, 10);
    create_macro_panel(terminal, terminal->macro_panel);

    // Create center panel
    GtkWidget *center_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_box_pack_start(GTK_BOX(terminal->main_hbox), center_panel, TRUE, TRUE, 15);

    // Create data area in center
    create_data_area(terminal, center_panel);

    // Create hidden appearance and display options panels for menu dialogs
    // These widgets are needed for the menu callbacks but not displayed in the main UI
    GtkWidget *hidden_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    create_appearance_panel(terminal, hidden_container);
    create_display_options_panel(terminal, hidden_container);
    // Don't add hidden_container to any visible parent

    // Create status bar
    GtkWidget *status_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(main_vbox), status_hbox, FALSE, FALSE, 0);

    terminal->status_label = gtk_label_new("Disconnected");
    gtk_widget_set_halign(terminal->status_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(status_hbox), terminal->status_label, TRUE, TRUE, 5);

    terminal->stats_label = gtk_label_new("Sent: 0 | Received: 0 | Time: 00:00:00");
    gtk_box_pack_start(GTK_BOX(status_hbox), terminal->stats_label, FALSE, FALSE, 5);
}

