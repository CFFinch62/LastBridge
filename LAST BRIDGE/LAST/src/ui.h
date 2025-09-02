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

// Panel creation functions
void create_connection_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_display_options_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_appearance_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_file_operations_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_control_signals_panel(SerialTerminal *terminal, GtkWidget *parent);
void create_data_area(SerialTerminal *terminal, GtkWidget *parent);

// Appearance functions
void apply_appearance_settings(SerialTerminal *terminal);
void apply_theme_setting(SerialTerminal *terminal);

#endif // UI_H
