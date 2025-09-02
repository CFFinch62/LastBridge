#ifndef SETTINGS_H
#define SETTINGS_H

#include "common.h"

/*
 * Settings persistence module for LAST
 * Handles loading and saving of application settings
 */

// Settings file functions
void load_settings(SerialTerminal *terminal);
void save_settings(SerialTerminal *terminal);
char* get_config_file_path(void);

// UI synchronization functions
void apply_loaded_settings(SerialTerminal *terminal);
void update_settings_from_ui(SerialTerminal *terminal);

#endif // SETTINGS_H
