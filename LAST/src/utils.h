#ifndef UTILS_H
#define UTILS_H

#include "common.h"

/*
 * Utility functions module for LAST
 * Handles data formatting, timestamps, and statistics
 */

// Data formatting functions
char* format_data_for_display(const char *data, size_t data_len, gboolean hex_mode);
char* get_current_timestamp(void);

// Statistics functions
void update_statistics(SerialTerminal *terminal);

// Timer callback for statistics updates
gboolean update_statistics_timer(gpointer data);

// Macro chaining structures and functions
typedef struct {
    char **parts;           // Array of command parts
    gboolean *is_macro_ref; // Array indicating if part is from macro reference
    int count;              // Number of parts
} MacroParts;

char* expand_macro_references(SerialTerminal *terminal, const char *command, int current_macro_index);
MacroParts* parse_macro_command(SerialTerminal *terminal, const char *command, int current_macro_index);
void free_macro_parts(MacroParts *parts);
int resolve_macro_by_name(SerialTerminal *terminal, const char *name);
int resolve_macro_by_number(const char *number_str);
gboolean has_macro_reference(const char *command);

#endif // UTILS_H
