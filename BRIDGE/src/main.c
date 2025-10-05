/*
 * Main application file for BRIDGE - Virtual Null Modem Bridge
 * Coordinates all modules and handles application initialization
 */

#include "common.h"
#include "nullmodem.h"
#include "sniffing.h"
#include "ui.h"
#include "utils.h"
#include "settings.h"
#include "callbacks.h"

// Global application instance (defined here, declared in common.h)
BridgeApp *g_bridge_app = NULL;

void cleanup_and_exit(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    if (g_bridge_app) {
        printf("\nReceived signal, shutting down...\n");
        
        // Save settings
        save_settings(g_bridge_app);

        // Stop sniffing if active
        cleanup_sniffing(g_bridge_app);

        // Stop null modem if running
        if (g_bridge_app->state == BRIDGE_STATE_RUNNING) {
            stop_null_modem(g_bridge_app);
        }
        
        // Stop status timer
        if (g_bridge_app->status_timer_id > 0) {
            g_source_remove(g_bridge_app->status_timer_id);
        }
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    // Create and initialize application structure
    BridgeApp app = {0};
    g_bridge_app = &app;
    
    // Set up signal handlers
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);
    
    // Initialize application state
    app.state = BRIDGE_STATE_STOPPED;
    app.socat_pid = 0;
    app.running = FALSE;
    app.monitor_running = FALSE;
    app.status_timer_id = 0;
    
    // Initialize default settings
    init_default_settings(&app);

    // Initialize sniffing
    init_sniffing(&app);

    // Load saved settings
    load_settings(&app);
    
    // Create main window and UI
    create_main_window(&app);
    
    // Apply loaded settings to UI widgets BEFORE connecting signals
    apply_loaded_settings(&app);
    
    // Apply appearance settings
    apply_appearance_settings(&app);
    
    // Connect all signals AFTER settings are applied
    connect_signals(&app);
    
    // Start status update timer (every second)
    app.status_timer_id = g_timeout_add(1000, update_status_timer, &app);
    
    // Initial UI state update
    update_ui_state(&app);
    
    // Show window
    gtk_widget_show_all(app.window);
    
    // Auto-start if configured
    if (app.auto_start) {
        log_message(&app, "Auto-starting null modem...");
        create_null_modem(&app);
    }
    
    // Log startup message
    log_message(&app, "BRIDGE - Virtual Null Modem Bridge started");
    log_message(&app, "Ready to create virtual null modem devices");
    
    // Run main loop
    gtk_main();
    
    // Cleanup
    if (app.device_permissions) g_free(app.device_permissions);
    if (app.font_family) g_free(app.font_family);
    if (app.bg_color) g_free(app.bg_color);
    if (app.text_color) g_free(app.text_color);
    if (app.theme_preference) g_free(app.theme_preference);
    
    return 0;
}
