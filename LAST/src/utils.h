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

#endif // UTILS_H
