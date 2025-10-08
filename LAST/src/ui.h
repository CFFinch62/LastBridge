#ifndef UI_H
#define UI_H

#include "common.h"

/*
 * User Interface module for LAST
 * Handles GTK UI creation and layout
 */

// Main interface creation
void create_main_interface(SerialTerminal *terminal);
void create_menu_bar(SerialTerminal *terminal, GtkWidget *parent);
void create_scripting_window(SerialTerminal *terminal);

// Panel creation functions
void create_connection_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_display_options_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_appearance_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_file_operations_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_control_signals_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_signal_indicators(SerialTerminal *terminal, GtkWidget *parent);
void create_data_area(SerialTerminal *terminal, GtkWidget *parent);

// Appearance functions
void apply_appearance_settings(SerialTerminal *terminal);
void apply_theme_setting(SerialTerminal *terminal);

// Interval dropdown functions
void populate_interval_dropdown_for_repeat(SerialTerminal *terminal);
void populate_interval_dropdown_for_lines(SerialTerminal *terminal);
void update_interval_dropdown_based_on_mode(SerialTerminal *terminal);

// Macro functions
void create_macro_panel(SerialTerminal *terminal, GtkWidget *parent);
void show_macro_programming_dialog(SerialTerminal *terminal);
void toggle_macro_panel_visibility(SerialTerminal *terminal);

// Scripting functions
void create_script_window(SerialTerminal *terminal);
void show_script_window(SerialTerminal *terminal);

#endif // UI_H
