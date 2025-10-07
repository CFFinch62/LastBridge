/*
 * Utility functions module for LAST - Linux Advanced Serial Transceiver
 * Handles data formatting, timestamps, and statistics
 */

#include "utils.h"

char* format_data_for_display(const char *data, size_t data_len, gboolean hex_mode) {
    if (!hex_mode) {
        // In text mode, create a null-terminated copy of the data
        char *text_copy = malloc(data_len + 1);
        memcpy(text_copy, data, data_len);
        text_copy[data_len] = '\0';
        return text_copy;
    }

    // Get hex bytes per line setting from global terminal (extern in common.h)
    extern SerialTerminal *g_terminal;
    int bytes_per_line = (g_terminal && g_terminal->hex_bytes_per_line > 0) ?
                         g_terminal->hex_bytes_per_line : 0;

    // Convert to hex display
    // Allocate extra space for potential newlines and null terminator
    char *hex_str = malloc(data_len * 4 + 100); // Extra space for newlines and formatting
    hex_str[0] = '\0';

    if (bytes_per_line == 0) {
        // Mode 1: Line breaks at CR LF sequences (0D 0A)
        for (size_t i = 0; i < data_len; i++) {
            char hex_byte[4];
            unsigned char byte = (unsigned char)data[i];
            snprintf(hex_byte, sizeof(hex_byte), "%02X ", byte);
            strcat(hex_str, hex_byte);

            // Check for CR LF sequence (0D 0A) to create line breaks
            if (byte == 0x0D && i + 1 < data_len && (unsigned char)data[i + 1] == 0x0A) {
                // Add the LF byte
                i++;
                snprintf(hex_byte, sizeof(hex_byte), "%02X", (unsigned char)data[i]);
                strcat(hex_str, hex_byte);
                // Add a newline to display for better readability
                strcat(hex_str, "\n");
            }
        }
    } else {
        // Mode 2: Fixed number of bytes per line
        for (size_t i = 0; i < data_len; i++) {
            char hex_byte[4];
            unsigned char byte = (unsigned char)data[i];
            snprintf(hex_byte, sizeof(hex_byte), "%02X ", byte);
            strcat(hex_str, hex_byte);

            // Add newline after specified number of bytes
            if ((i + 1) % bytes_per_line == 0) {
                strcat(hex_str, "\n");
            }
        }
    }

    return hex_str;
}

char* get_current_timestamp(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char *timestamp = malloc(32);
    strftime(timestamp, 32, "%Y-%m-%d %H:%M:%S", tm_info);
    return timestamp;
}

void update_statistics(SerialTerminal *terminal) {
    if (!terminal->stats_label) return;

    char stats_text[256];

    if (terminal->connected) {
        time_t current_time = time(NULL);
        int connection_duration = (int)(current_time - terminal->connection_start_time);
        int hours = connection_duration / 3600;
        int minutes = (connection_duration % 3600) / 60;
        int seconds = connection_duration % 60;

        snprintf(stats_text, sizeof(stats_text),
                "Sent: %lu bytes | Received: %lu bytes | Time: %02d:%02d:%02d",
                terminal->bytes_sent, terminal->bytes_received, hours, minutes, seconds);
    } else {
        snprintf(stats_text, sizeof(stats_text),
                "Sent: %lu bytes | Received: %lu bytes | Time: 00:00:00",
                terminal->bytes_sent, terminal->bytes_received);
    }

    gtk_label_set_text(GTK_LABEL(terminal->stats_label), stats_text);
}

gboolean update_statistics_timer(gpointer data) {
    update_statistics((SerialTerminal *)data);
    return TRUE; // Continue timer
}

// Macro chaining implementation

gboolean has_macro_reference(const char *command) {
    if (!command) return FALSE;

    const char *start = strchr(command, '{');
    if (!start) return FALSE;

    const char *end = strchr(start, '}');
    return (end != NULL && end > start + 1);
}

int resolve_macro_by_number(const char *number_str) {
    if (!number_str) return -1;

    int macro_num = atoi(number_str);
    // Convert 1-based user numbering to 0-based array index
    if (macro_num >= 1 && macro_num <= MAX_MACRO_BUTTONS) {
        return macro_num - 1;
    }
    return -1;
}

int resolve_macro_by_name(SerialTerminal *terminal, const char *name) {
    if (!terminal || !name) return -1;

    // First try to resolve as a number
    int by_number = resolve_macro_by_number(name);
    if (by_number >= 0) return by_number;

    // Then try to match by label name
    for (int i = 0; i < MAX_MACRO_BUTTONS; i++) {
        if (strcmp(terminal->macro_labels[i], name) == 0) {
            return i;
        }
    }

    // Finally try to match "macro1", "macro2", etc. format
    if (strncmp(name, "macro", 5) == 0) {
        return resolve_macro_by_number(name + 5);
    }

    return -1;
}

MacroParts* parse_macro_command(SerialTerminal *terminal, const char *command, int current_macro_index) {
    if (!terminal || !command) return NULL;

    MacroParts *parts = malloc(sizeof(MacroParts));
    if (!parts) return NULL;

    parts->parts = malloc(sizeof(char*) * 32); // Max 32 parts
    parts->is_macro_ref = malloc(sizeof(gboolean) * 32);
    parts->count = 0;

    if (!parts->parts || !parts->is_macro_ref) {
        if (parts->parts) free(parts->parts);
        if (parts->is_macro_ref) free(parts->is_macro_ref);
        free(parts);
        return NULL;
    }

    // Track recursion to prevent infinite loops
    static int recursion_depth = 0;
    static gboolean recursion_stack[MAX_MACRO_BUTTONS];

    // Initialize recursion tracking on first call
    if (recursion_depth == 0) {
        memset(recursion_stack, FALSE, sizeof(recursion_stack));
    }

    // Prevent infinite recursion
    if (recursion_depth > 10) {
        // Add original command as single part
        parts->parts[0] = strdup(command);
        parts->is_macro_ref[0] = FALSE;
        parts->count = 1;
        return parts;
    }

    // Mark current macro as being processed
    if (current_macro_index >= 0 && current_macro_index < MAX_MACRO_BUTTONS) {
        if (recursion_stack[current_macro_index]) {
            // Circular reference - add original command as single part
            parts->parts[0] = strdup(command);
            parts->is_macro_ref[0] = FALSE;
            parts->count = 1;
            return parts;
        }
        recursion_stack[current_macro_index] = TRUE;
    }

    recursion_depth++;

    const char *pos = command;

    while (*pos && parts->count < 32) {
        const char *start = strchr(pos, '{');
        if (!start) {
            // No more references, add rest as regular text if not empty
            if (*pos) {
                parts->parts[parts->count] = strdup(pos);
                parts->is_macro_ref[parts->count] = FALSE;
                parts->count++;
            }
            break;
        }

        // Add text before the reference as regular part
        if (start > pos) {
            size_t prefix_len = start - pos;
            char *prefix = malloc(prefix_len + 1);
            if (prefix) {
                strncpy(prefix, pos, prefix_len);
                prefix[prefix_len] = '\0';
                parts->parts[parts->count] = prefix;
                parts->is_macro_ref[parts->count] = FALSE;
                parts->count++;
            }
        }

        // Find the closing brace
        const char *end = strchr(start, '}');
        if (!end) {
            // Malformed reference, add rest as regular text
            parts->parts[parts->count] = strdup(start);
            parts->is_macro_ref[parts->count] = FALSE;
            parts->count++;
            break;
        }

        // Extract macro name/number
        size_t name_len = end - start - 1;
        if (name_len > 0 && name_len < 32) {
            char macro_name[32];
            strncpy(macro_name, start + 1, name_len);
            macro_name[name_len] = '\0';

            // Resolve macro reference
            int macro_index = resolve_macro_by_name(terminal, macro_name);
            if (macro_index >= 0 && macro_index < MAX_MACRO_BUTTONS) {
                const char *macro_command = terminal->macro_commands[macro_index];
                if (strlen(macro_command) > 0) {
                    // Recursively parse the referenced macro
                    MacroParts *sub_parts = parse_macro_command(terminal, macro_command, macro_index);
                    if (sub_parts) {
                        // Add all sub-parts to our parts list
                        for (int i = 0; i < sub_parts->count && parts->count < 32; i++) {
                            parts->parts[parts->count] = strdup(sub_parts->parts[i]);
                            parts->is_macro_ref[parts->count] = TRUE; // Mark as from macro reference
                            parts->count++;
                        }
                        // Free sub_parts
                        for (int i = 0; i < sub_parts->count; i++) {
                            free(sub_parts->parts[i]);
                        }
                        free(sub_parts->parts);
                        free(sub_parts->is_macro_ref);
                        free(sub_parts);
                    }
                }
            } else {
                // Unknown macro reference, keep as regular text
                char *ref_text = malloc(name_len + 3);
                if (ref_text) {
                    snprintf(ref_text, name_len + 3, "{%s}", macro_name);
                    parts->parts[parts->count] = ref_text;
                    parts->is_macro_ref[parts->count] = FALSE;
                    parts->count++;
                }
            }
        } else {
            // Empty or too long reference, keep as regular text
            size_t ref_len = end - start + 1;
            char *ref_text = malloc(ref_len + 1);
            if (ref_text) {
                strncpy(ref_text, start, ref_len);
                ref_text[ref_len] = '\0';
                parts->parts[parts->count] = ref_text;
                parts->is_macro_ref[parts->count] = FALSE;
                parts->count++;
            }
        }

        pos = end + 1;
    }

    // Clean up recursion tracking
    recursion_depth--;
    if (current_macro_index >= 0 && current_macro_index < MAX_MACRO_BUTTONS) {
        recursion_stack[current_macro_index] = FALSE;
    }

    return parts;
}

void free_macro_parts(MacroParts *parts) {
    if (!parts) return;

    for (int i = 0; i < parts->count; i++) {
        if (parts->parts[i]) {
            free(parts->parts[i]);
        }
    }
    free(parts->parts);
    free(parts->is_macro_ref);
    free(parts);
}

char* expand_macro_references(SerialTerminal *terminal, const char *command, int current_macro_index) {
    if (!terminal || !command) return NULL;

    // Check if command has any macro references
    if (!has_macro_reference(command)) {
        return strdup(command); // Return copy of original command
    }

    // Parse command into parts
    MacroParts *parts = parse_macro_command(terminal, command, current_macro_index);
    if (!parts) return strdup(command);

    // Calculate total length needed
    size_t total_len = 1; // For null terminator
    for (int i = 0; i < parts->count; i++) {
        if (parts->parts[i]) {
            total_len += strlen(parts->parts[i]);
        }
    }

    // Allocate and build expanded string
    char *expanded = malloc(total_len);
    if (!expanded) {
        free_macro_parts(parts);
        return strdup(command);
    }

    expanded[0] = '\0';
    for (int i = 0; i < parts->count; i++) {
        if (parts->parts[i]) {
            strcat(expanded, parts->parts[i]);
        }
    }

    free_macro_parts(parts);
    return expanded;
}
