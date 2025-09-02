/*
 * Settings module header for BRIDGE - Virtual Null Modem Bridge
 * Configuration persistence and management
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include "common.h"

// Function declarations
void load_settings(BridgeApp *app);
void save_settings(BridgeApp *app);
void apply_appearance_settings(BridgeApp *app);
void apply_loaded_settings(BridgeApp *app);
char* get_config_file_path(void);
void init_default_settings(BridgeApp *app);

#endif // SETTINGS_H
