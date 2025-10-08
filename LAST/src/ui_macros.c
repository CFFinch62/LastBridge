/*
 * Menu bar, macros, and scripting window for LAST - Linux Advanced Serial Terminal
 * Handles menu bar creation, macro panel, and scripting window
 */

#include "ui.h"
#include "callbacks.h"
#include "settings.h"

void create_menu_bar(SerialTerminal *terminal, GtkWidget *parent) {
    // Create menu bar
    terminal->menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(parent), terminal->menu_bar, FALSE, FALSE, 0);

    // File menu
    GtkWidget *file_menu_item = gtk_menu_item_new_with_label("File");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), file_menu_item);

    terminal->file_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), terminal->file_menu);

    GtkWidget *exit_item = gtk_menu_item_new_with_label("Exit");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->file_menu), exit_item);
    g_signal_connect(exit_item, "activate", G_CALLBACK(on_file_exit_activate), terminal);

    // View menu
    GtkWidget *view_menu_item = gtk_menu_item_new_with_label("View");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), view_menu_item);

    terminal->view_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_item), terminal->view_menu);

    GtkWidget *appearance_item = gtk_menu_item_new_with_label("Appearance...");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->view_menu), appearance_item);
    g_signal_connect(appearance_item, "activate", G_CALLBACK(on_view_appearance_activate), terminal);

    GtkWidget *display_options_item = gtk_menu_item_new_with_label("Display Options...");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->view_menu), display_options_item);
    g_signal_connect(display_options_item, "activate", G_CALLBACK(on_view_display_options_activate), terminal);

    // Tools menu
    GtkWidget *tools_menu_item = gtk_menu_item_new_with_label("Tools");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), tools_menu_item);

    terminal->tools_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools_menu_item), terminal->tools_menu);

    GtkWidget *bridge_item = gtk_menu_item_new_with_label("BRIDGE");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->tools_menu), bridge_item);
    g_signal_connect(bridge_item, "activate", G_CALLBACK(on_tools_bridge_activate), terminal);

    GtkWidget *scripting_item = gtk_menu_item_new_with_label("Lua Scripting...");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->tools_menu), scripting_item);
    g_signal_connect(scripting_item, "activate", G_CALLBACK(on_tools_scripting_activate), terminal);

    // Macros menu
    GtkWidget *macros_menu_item = gtk_menu_item_new_with_label("Macros");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), macros_menu_item);

    terminal->macros_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(macros_menu_item), terminal->macros_menu);

    GtkWidget *program_macros_item = gtk_menu_item_new_with_label("Program Macros...");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->macros_menu), program_macros_item);
    g_signal_connect(program_macros_item, "activate", G_CALLBACK(on_macros_program_activate), terminal);

    // Add separator
    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->macros_menu), separator);

    // Add show/hide toggle
    GtkWidget *toggle_macros_item = gtk_menu_item_new_with_label("Show/Hide Macro Panel");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->macros_menu), toggle_macros_item);
    g_signal_connect(toggle_macros_item, "activate", G_CALLBACK(on_macros_toggle_activate), terminal);

    // Help menu
    GtkWidget *help_menu_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->menu_bar), help_menu_item);

    terminal->help_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), terminal->help_menu);

    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(terminal->help_menu), about_item);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_help_about_activate), terminal);
}

void populate_interval_dropdown_for_repeat(SerialTerminal *terminal) {
    // Clear existing items
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo));

    // Add repeat mode intervals (seconds)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.2");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "0.5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "1.0");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "2.0");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "5.0");

    // Set default to 1.0 second
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->send_file_interval_combo), 3);
}

void populate_interval_dropdown_for_lines(SerialTerminal *terminal) {
    // Clear existing items
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo));

    // Add line-by-line mode intervals (milliseconds)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "10");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "25");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "50");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "100");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "200");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "250");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "500");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(terminal->send_file_interval_combo), "1000");

    // Set default to 100ms
    gtk_combo_box_set_active(GTK_COMBO_BOX(terminal->send_file_interval_combo), 4);
}

void update_interval_dropdown_based_on_mode(SerialTerminal *terminal) {
    gboolean lines_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal->send_file_lines_check));

    if (lines_mode) {
        populate_interval_dropdown_for_lines(terminal);
        gtk_label_set_text(GTK_LABEL(terminal->send_file_interval_label), "ms");
    } else {
        populate_interval_dropdown_for_repeat(terminal);
        gtk_label_set_text(GTK_LABEL(terminal->send_file_interval_label), "sec");
    }
}

void create_macro_panel(SerialTerminal *terminal, GtkWidget *parent) {
    // Create frame for macros
    GtkWidget *frame = gtk_frame_new("Macros");
    gtk_box_pack_start(GTK_BOX(parent), frame, TRUE, TRUE, 5);

    // Create scrolled window for macro buttons
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(frame), scrolled);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled), 5);

    // Create vertical box for macro buttons
    GtkWidget *macro_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_add(GTK_CONTAINER(scrolled), macro_vbox);

    // Create macro buttons
    for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
        char default_label[32];
        snprintf(default_label, sizeof(default_label), "Macro %d", i + 1);

        terminal->macro_buttons[i] = gtk_button_new_with_label(default_label);
        gtk_box_pack_start(GTK_BOX(macro_vbox), terminal->macro_buttons[i], FALSE, FALSE, 2);

        // Set button size
        gtk_widget_set_size_request(terminal->macro_buttons[i], 180, 30);

        // Store button index as data for callback
        g_object_set_data(G_OBJECT(terminal->macro_buttons[i]), "macro_index", GINT_TO_POINTER(i));

        // Connect signal
        g_signal_connect(terminal->macro_buttons[i], "clicked", G_CALLBACK(on_macro_button_clicked), terminal);

        // Initialize macro data
        strncpy(terminal->macro_labels[i], default_label, MAX_MACRO_LABEL_LENGTH - 1);
        terminal->macro_labels[i][MAX_MACRO_LABEL_LENGTH - 1] = '\0';
        terminal->macro_commands[i][0] = '\0'; // Empty command initially
    }
}

void show_macro_programming_dialog(SerialTerminal *terminal) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Program Macros",
                                                   GTK_WINDOW(terminal->window),
                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_OK", GTK_RESPONSE_OK,
                                                   NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 600);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);

    // Create scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(content_area), scrolled, TRUE, TRUE, 0);

    // Create grid for macro entries
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(scrolled), grid);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    // Create entry widgets for each macro
    GtkWidget *label_entries[MAX_MACRO_BUTTONS];
    GtkWidget *command_entries[MAX_MACRO_BUTTONS];

    // Add headers
    GtkWidget *header_macro = gtk_label_new("Macro");
    GtkWidget *header_label = gtk_label_new("Button Label");
    GtkWidget *header_command = gtk_label_new("Command to Send");

    gtk_widget_set_halign(header_macro, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(header_label, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(header_command, GTK_ALIGN_CENTER);

    // Make headers bold
    PangoAttrList *attrs = pango_attr_list_new();
    PangoAttribute *bold = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attrs, bold);
    gtk_label_set_attributes(GTK_LABEL(header_macro), attrs);
    gtk_label_set_attributes(GTK_LABEL(header_label), pango_attr_list_copy(attrs));
    gtk_label_set_attributes(GTK_LABEL(header_command), pango_attr_list_copy(attrs));
    pango_attr_list_unref(attrs);

    gtk_grid_attach(GTK_GRID(grid), header_macro, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_command, 2, 0, 1, 1);

    for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
        // Macro number label
        char macro_num[16];
        snprintf(macro_num, sizeof(macro_num), "%d", i + 1);
        GtkWidget *num_label = gtk_label_new(macro_num);
        gtk_widget_set_halign(num_label, GTK_ALIGN_CENTER);
        gtk_grid_attach(GTK_GRID(grid), num_label, 0, i + 1, 1, 1);

        // Label entry
        label_entries[i] = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(label_entries[i]), terminal->macro_labels[i]);
        gtk_entry_set_max_length(GTK_ENTRY(label_entries[i]), MAX_MACRO_LABEL_LENGTH - 1);
        gtk_widget_set_size_request(label_entries[i], 120, -1);
        gtk_grid_attach(GTK_GRID(grid), label_entries[i], 1, i + 1, 1, 1);

        // Command entry
        command_entries[i] = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(command_entries[i]), terminal->macro_commands[i]);
        gtk_entry_set_max_length(GTK_ENTRY(command_entries[i]), MAX_MACRO_COMMAND_LENGTH - 1);
        gtk_widget_set_size_request(command_entries[i], 250, -1);
        gtk_grid_attach(GTK_GRID(grid), command_entries[i], 2, i + 1, 1, 1);
    }

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        // Save the macro settings
        for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
            const char *label = gtk_entry_get_text(GTK_ENTRY(label_entries[i]));
            const char *command = gtk_entry_get_text(GTK_ENTRY(command_entries[i]));

            strncpy(terminal->macro_labels[i], label, MAX_MACRO_LABEL_LENGTH - 1);
            terminal->macro_labels[i][MAX_MACRO_LABEL_LENGTH - 1] = '\0';

            strncpy(terminal->macro_commands[i], command, MAX_MACRO_COMMAND_LENGTH - 1);
            terminal->macro_commands[i][MAX_MACRO_COMMAND_LENGTH - 1] = '\0';

            // Update button label
            gtk_button_set_label(GTK_BUTTON(terminal->macro_buttons[i]), terminal->macro_labels[i]);
        }

        // Save settings to file
        save_settings(terminal);
    }

    gtk_widget_destroy(dialog);
}

void toggle_macro_panel_visibility(SerialTerminal *terminal) {
    // Get current window size before toggling
    int current_width, current_height;
    gtk_window_get_size(GTK_WINDOW(terminal->window), &current_width, &current_height);

    if (terminal->macro_panel_visible) {
        // Hide the macro panel
        gtk_widget_hide(terminal->macro_panel);
        terminal->macro_panel_visible = FALSE;
    } else {
        // Show the macro panel
        gtk_widget_show(terminal->macro_panel);
        terminal->macro_panel_visible = TRUE;
    }

    // Force window to maintain its current size
    // This prevents the window from shrinking when macro panel is hidden
    gtk_window_resize(GTK_WINDOW(terminal->window), current_width, current_height);

    // Save the visibility state
    save_settings(terminal);
}

void create_scripting_window(SerialTerminal *terminal) {
    // Don't create multiple windows
    if (terminal->script_window) {
        gtk_window_present(GTK_WINDOW(terminal->script_window));
        return;
    }

    // Create scripting window
    terminal->script_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(terminal->script_window), "Lua Scripting Engine");
    gtk_window_set_default_size(GTK_WINDOW(terminal->script_window), 800, 600);
    gtk_window_set_transient_for(GTK_WINDOW(terminal->script_window), GTK_WINDOW(terminal->window));
    gtk_window_set_position(GTK_WINDOW(terminal->script_window), GTK_WIN_POS_CENTER_ON_PARENT);

    // Create main container
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(terminal->script_window), main_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 10);

    // Create toolbar
    GtkWidget *toolbar_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), toolbar_hbox, FALSE, FALSE, 0);

    // Enable/Disable scripting checkbox
    terminal->script_enable_check = gtk_check_button_new_with_label("Enable Scripting");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(terminal->script_enable_check), terminal->scripting_enabled);
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), terminal->script_enable_check, FALSE, FALSE, 0);

    // Add spacer
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), spacer, TRUE, TRUE, 0);

    // Load script button
    terminal->script_load_button = gtk_button_new_with_label("Load Script...");
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), terminal->script_load_button, FALSE, FALSE, 0);

    // Save script button
    terminal->script_save_button = gtk_button_new_with_label("Save Script...");
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), terminal->script_save_button, FALSE, FALSE, 0);

    // Test script button
    terminal->script_test_button = gtk_button_new_with_label("Test Script");
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), terminal->script_test_button, FALSE, FALSE, 0);

    // Clear script button
    terminal->script_clear_button = gtk_button_new_with_label("Clear");
    gtk_box_pack_start(GTK_BOX(toolbar_hbox), terminal->script_clear_button, FALSE, FALSE, 0);

    // Create script editor area
    GtkWidget *editor_frame = gtk_frame_new("Lua Script Editor");
    gtk_box_pack_start(GTK_BOX(main_vbox), editor_frame, TRUE, TRUE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(editor_frame), scrolled);

    terminal->script_text_view = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(terminal->script_text_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(terminal->script_text_view), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(scrolled), terminal->script_text_view);

    // Set initial script content if available
    if (terminal->script_content) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(terminal->script_text_view));
        gtk_text_buffer_set_text(buffer, terminal->script_content, -1);
    }

    // Create info area
    GtkWidget *info_frame = gtk_frame_new("Script Information");
    gtk_box_pack_start(GTK_BOX(main_vbox), info_frame, FALSE, FALSE, 0);

    GtkWidget *info_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(info_frame), info_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(info_vbox), 10);

    GtkWidget *info_label = gtk_label_new(
        "Available Functions:\n"
        "• on_data_received(data) - Called when data is received\n"
        "• on_data_send(data) - Called before data is sent\n"
        "• on_connection_open() - Called when connection opens\n"
        "• on_connection_close() - Called when connection closes\n\n"
        "Available API Functions:\n"
        "• log(message) - Add message to terminal log\n"
        "• send(data) - Send data through connection\n"
        "• get_connection_info() - Get connection details\n"
        "• get_statistics() - Get connection statistics\n"
        "• parse_nmea(sentence) - Parse NMEA sentence\n"
        "• create_nmea(talker, sentence, data) - Create NMEA sentence\n"
        "• calculate_checksum(data) - Calculate NMEA checksum"
    );
    gtk_label_set_justify(GTK_LABEL(info_label), GTK_JUSTIFY_LEFT);
    gtk_widget_set_halign(info_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(info_vbox), info_label, FALSE, FALSE, 0);

    // Connect window destroy signal
    g_signal_connect(terminal->script_window, "destroy", G_CALLBACK(on_script_window_destroy), terminal);

    // Connect button signals
    g_signal_connect(terminal->script_enable_check, "toggled", G_CALLBACK(on_script_enable_toggled), terminal);
    g_signal_connect(terminal->script_load_button, "clicked", G_CALLBACK(on_script_load_clicked), terminal);
    g_signal_connect(terminal->script_save_button, "clicked", G_CALLBACK(on_script_save_clicked), terminal);
    g_signal_connect(terminal->script_test_button, "clicked", G_CALLBACK(on_script_test_clicked), terminal);
    g_signal_connect(terminal->script_clear_button, "clicked", G_CALLBACK(on_script_clear_clicked), terminal);

    // Show the window
    gtk_widget_show_all(terminal->script_window);
}

