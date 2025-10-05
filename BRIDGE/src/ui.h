/*
 * User interface module header for BRIDGE - Virtual Null Modem Bridge
 * GTK3 interface creation and management
 */

#ifndef UI_H
#define UI_H

#include "common.h"

// Function declarations
void create_main_window(BridgeApp *app);
gboolean update_ui_state(gpointer data);
void append_log_message(BridgeApp *app, const char *message, gboolean timestamp);
void clear_log(BridgeApp *app);
void create_configuration_tab(BridgeApp *app, GtkWidget *notebook);
void create_status_tab(BridgeApp *app, GtkWidget *notebook);
void create_sniffing_tab(BridgeApp *app, GtkWidget *notebook);
void create_settings_tab(BridgeApp *app, GtkWidget *notebook);
void apply_ui_theme(BridgeApp *app);

#endif // UI_H
