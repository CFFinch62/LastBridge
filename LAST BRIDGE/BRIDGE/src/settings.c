/*
 * Settings module for BRIDGE - Virtual Null Modem Bridge
 * Configuration persistence and management
 */

#include "settings.h"
#include "utils.h"

char* get_config_file_path(void) {
    const char *home = getenv("HOME");
    if (!home) {
        home = "/tmp";
    }
    
    char *config_path = malloc(256);
    snprintf(config_path, 256, "%s/.bridge_config", home);
    return config_path;
}

void init_default_settings(BridgeApp *app) {
    // Initialize default settings
    strncpy(app->device1_path, DEFAULT_DEVICE1, MAX_PATH_LENGTH - 1);
    strncpy(app->device2_path, DEFAULT_DEVICE2, MAX_PATH_LENGTH - 1);
    
    app->auto_start = FALSE;
    app->verbose_logging = FALSE;
    app->device_permissions = g_strdup("666");

    // Appearance settings (following LAST pattern)
    app->font_family = g_strdup("Monospace");
    app->font_size = 10;
    app->bg_color = g_strdup("#FFFFFF");
    app->text_color = g_strdup("#000000");
    app->theme_preference = g_strdup("system");
    
    // Initialize statistics
    app->test_count = 0;
    app->successful_tests = 0;
    app->last_test_time = 0;
}

void load_settings(BridgeApp *app) {
    char *config_path = get_config_file_path();
    FILE *file = fopen(config_path, "r");
    
    if (!file) {
        free(config_path);
        return; // Use defaults if no config file
    }
    
    char line[512];
    char key[256], value[256];
    
    while (fgets(line, sizeof(line), file)) {
        // Skip comments, empty lines, and section headers
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r' || line[0] == '[') continue;

        if (sscanf(line, "%255[^=]=%255[^\n\r]", key, value) == 2) {
            // Device settings
            if (strcmp(key, "device1_path") == 0) {
                strncpy(app->device1_path, value, MAX_PATH_LENGTH - 1);
            } else if (strcmp(key, "device2_path") == 0) {
                strncpy(app->device2_path, value, MAX_PATH_LENGTH - 1);
            } else if (strcmp(key, "auto_start") == 0) {
                app->auto_start = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "verbose_logging") == 0) {
                app->verbose_logging = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "device_permissions") == 0) {
                if (app->device_permissions) g_free(app->device_permissions);
                app->device_permissions = g_strdup(value);
            }
            // Appearance settings
            else if (strcmp(key, "theme") == 0) {
                if (app->theme_preference) g_free(app->theme_preference);
                app->theme_preference = g_strdup(value);
            } else if (strcmp(key, "font_family") == 0) {
                if (app->font_family) g_free(app->font_family);
                app->font_family = g_strdup(value);
            } else if (strcmp(key, "font_size") == 0) {
                app->font_size = atoi(value);
            } else if (strcmp(key, "bg_color") == 0) {
                if (app->bg_color) g_free(app->bg_color);
                app->bg_color = g_strdup(value);
            } else if (strcmp(key, "text_color") == 0) {
                if (app->text_color) g_free(app->text_color);
                app->text_color = g_strdup(value);
            }
        }
    }

    fclose(file);
    free(config_path);
}

void save_settings(BridgeApp *app) {
    char *config_path = get_config_file_path();
    FILE *file = fopen(config_path, "w");
    
    if (!file) {
        free(config_path);
        return;
    }
    
    fprintf(file, "# BRIDGE - Virtual Null Modem Bridge Configuration\n");
    fprintf(file, "# Generated automatically - edit with care\n\n");
    
    // Appearance settings
    fprintf(file, "[Appearance]\n");
    fprintf(file, "theme=%s\n", app->theme_preference ? app->theme_preference : "system");
    fprintf(file, "font_family=%s\n", app->font_family ? app->font_family : "Monospace");
    fprintf(file, "font_size=%d\n", app->font_size);
    fprintf(file, "bg_color=%s\n", app->bg_color ? app->bg_color : "#FFFFFF");
    fprintf(file, "text_color=%s\n", app->text_color ? app->text_color : "#000000");
    fprintf(file, "\n");
    
    // Device settings
    fprintf(file, "[Device]\n");
    fprintf(file, "device1_path=%s\n", app->device1_path);
    fprintf(file, "device2_path=%s\n", app->device2_path);
    fprintf(file, "device_permissions=%s\n", app->device_permissions ? app->device_permissions : "666");
    fprintf(file, "\n");
    
    // Application settings
    fprintf(file, "[Application]\n");
    fprintf(file, "auto_start=%s\n", app->auto_start ? "true" : "false");
    fprintf(file, "verbose_logging=%s\n", app->verbose_logging ? "true" : "false");
    fprintf(file, "\n");
    
    fclose(file);
    free(config_path);
}

void apply_loaded_settings(BridgeApp *app) {
    // Apply device paths to UI
    if (app->device1_entry) {
        gtk_entry_set_text(GTK_ENTRY(app->device1_entry), app->device1_path);
    }
    if (app->device2_entry) {
        gtk_entry_set_text(GTK_ENTRY(app->device2_entry), app->device2_path);
    }
    
    // Apply checkboxes
    if (app->auto_start_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->auto_start_check), app->auto_start);
    }
    if (app->verbose_logging_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->verbose_logging_check), app->verbose_logging);
    }
    
    // Apply device permissions combo
    if (app->device_permissions_combo) {
        if (app->device_permissions && strcmp(app->device_permissions, "666") == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(app->device_permissions_combo), 1);
        } else if (app->device_permissions && strcmp(app->device_permissions, "644") == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(app->device_permissions_combo), 2);
        } else if (app->device_permissions && strcmp(app->device_permissions, "600") == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(app->device_permissions_combo), 3);
        } else {
            gtk_combo_box_set_active(GTK_COMBO_BOX(app->device_permissions_combo), 0);
        }
    }
}

void apply_appearance_settings(BridgeApp *app) {
    // Apply font and color settings to the log text view
    if (app->log_text && app->font_family) {
        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, app->font_family);
        pango_font_description_set_size(font_desc, app->font_size * PANGO_SCALE);
        
        gtk_widget_override_font(app->log_text, font_desc);
        pango_font_description_free(font_desc);
    }
    
    // Apply colors if available
    if (app->log_text && app->bg_color && app->text_color) {
        GdkRGBA bg_color, text_color;
        
        if (gdk_rgba_parse(&bg_color, app->bg_color)) {
            gtk_widget_override_background_color(app->log_text, GTK_STATE_FLAG_NORMAL, &bg_color);
        }
        
        if (gdk_rgba_parse(&text_color, app->text_color)) {
            gtk_widget_override_color(app->log_text, GTK_STATE_FLAG_NORMAL, &text_color);
        }
    }
}
