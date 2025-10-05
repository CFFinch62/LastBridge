/*
 * Null modem module for BRIDGE - Virtual Null Modem Bridge
 * Handles socat process management and virtual device operations
 */

#include "nullmodem.h"
#include "sniffing.h"
#include "utils.h"

gboolean check_socat_available(void) {
    int result = system("which socat > /dev/null 2>&1");
    return (result == 0);
}

gboolean create_null_modem(BridgeApp *app) {
    if (app->state == BRIDGE_STATE_RUNNING) {
        log_message(app, "Null modem is already running");
        return FALSE;
    }

    if (!check_socat_available()) {
        log_message(app, "ERROR: socat not found. Install with: sudo apt-get install socat");
        app->state = BRIDGE_STATE_ERROR;
        return FALSE;
    }

    app->state = BRIDGE_STATE_STARTING;
    update_ui_state(app);

    // Clean up any existing devices
    cleanup_devices(app);

    // Build socat command with optional data capture
    char cmd[1024];
    if (app->sniffing_enabled) {
        // Create temporary capture file for sniffing
        char capture_file[MAX_PATH_LENGTH];
        snprintf(capture_file, sizeof(capture_file), "/tmp/bridge_capture_%d.log", getpid());

        snprintf(cmd, sizeof(cmd),
            "socat -d -d -x pty,raw,echo=0,link=%s pty,raw,echo=0,link=%s 2>%s",
            app->device1_path, app->device2_path, capture_file);

        // Store capture file path for monitoring
        strncpy(app->capture_file_path, capture_file, MAX_PATH_LENGTH - 1);
    } else {
        snprintf(cmd, sizeof(cmd),
            "socat -d -d pty,raw,echo=0,link=%s pty,raw,echo=0,link=%s",
            app->device1_path, app->device2_path);
    }

    log_message(app, "Starting socat: %s", cmd);

    // Fork and exec socat
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - exec socat
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        exit(1); // Should never reach here
    } else if (pid > 0) {
        // Parent process
        app->socat_pid = pid;
        
        // Wait for devices to be created (with timeout)
        int timeout_ms = 0;
        while (timeout_ms < DEVICE_CREATION_TIMEOUT) {
            if (file_exists(app->device1_path) && file_exists(app->device2_path)) {
                break;
            }
            usleep(100000); // 100ms
            timeout_ms += 100;
        }

        if (!file_exists(app->device1_path) || !file_exists(app->device2_path)) {
            log_message(app, "ERROR: Devices not created within timeout");
            stop_null_modem(app);
            return FALSE;
        }

        // Check if socat is still running
        if (!is_process_running(app->socat_pid)) {
            log_message(app, "ERROR: socat process died unexpectedly");
            app->state = BRIDGE_STATE_ERROR;
            return FALSE;
        }

        // Set device permissions if configured
        set_device_permissions(app);

        app->state = BRIDGE_STATE_RUNNING;
        app->start_time = time(NULL);
        app->running = TRUE;

        // Start monitoring thread
        app->monitor_running = TRUE;
        pthread_create(&app->monitor_thread, NULL, monitor_socat_process, app);

        log_message(app, "✓ Created null modem: %s <-> %s", 
                   app->device1_path, app->device2_path);
        
        update_ui_state(app);
        return TRUE;
    } else {
        // Fork failed
        log_message(app, "ERROR: Failed to fork socat process: %s", strerror(errno));
        app->state = BRIDGE_STATE_ERROR;
        return FALSE;
    }
}

void stop_null_modem(BridgeApp *app) {
    if (app->state == BRIDGE_STATE_STOPPED) {
        return;
    }

    log_message(app, "Stopping null modem...");
    app->state = BRIDGE_STATE_STOPPING;
    app->running = FALSE;

    // Stop monitoring thread
    if (app->monitor_running) {
        app->monitor_running = FALSE;
        pthread_join(app->monitor_thread, NULL);
    }

    // Kill socat process
    if (app->socat_pid > 0) {
        safe_kill_process(app->socat_pid);
        app->socat_pid = 0;
    }

    // Clean up devices
    cleanup_devices(app);

    app->state = BRIDGE_STATE_STOPPED;
    log_message(app, "Null modem stopped");
    
    update_ui_state(app);
}

gboolean is_null_modem_running(BridgeApp *app) {
    if (app->state != BRIDGE_STATE_RUNNING) {
        return FALSE;
    }

    // Check if socat process is still alive
    if (app->socat_pid > 0 && !is_process_running(app->socat_pid)) {
        log_message(app, "WARNING: socat process died unexpectedly");
        app->state = BRIDGE_STATE_ERROR;
        return FALSE;
    }

    // Check if devices still exist
    if (!file_exists(app->device1_path) || !file_exists(app->device2_path)) {
        log_message(app, "WARNING: Devices disappeared");
        app->state = BRIDGE_STATE_ERROR;
        return FALSE;
    }

    return TRUE;
}

gboolean test_null_modem_communication(BridgeApp *app) {
    if (!is_null_modem_running(app)) {
        log_message(app, "Cannot test: null modem is not running");
        return FALSE;
    }

    app->test_count++;
    log_message(app, "Testing communication...");

    // Simple test: try to open both devices
    int fd1 = open(app->device1_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd1 < 0) {
        log_message(app, "✗ Failed to open %s: %s", app->device1_path, strerror(errno));
        return FALSE;
    }

    int fd2 = open(app->device2_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd2 < 0) {
        log_message(app, "✗ Failed to open %s: %s", app->device2_path, strerror(errno));
        close(fd1);
        return FALSE;
    }

    // Test data exchange
    const char *test_msg = "BRIDGE_TEST";
    ssize_t written = write(fd1, test_msg, strlen(test_msg));
    
    if (written != (ssize_t)strlen(test_msg)) {
        log_message(app, "✗ Failed to write test data");
        close(fd1);
        close(fd2);
        return FALSE;
    }

    // Give some time for data to transfer
    usleep(100000); // 100ms

    char buffer[64];
    ssize_t read_bytes = read(fd2, buffer, sizeof(buffer) - 1);
    
    close(fd1);
    close(fd2);

    if (read_bytes > 0) {
        buffer[read_bytes] = '\0';
        if (strncmp(buffer, test_msg, strlen(test_msg)) == 0) {
            app->successful_tests++;
            app->last_test_time = time(NULL);
            log_message(app, "✓ Communication test passed");
            return TRUE;
        }
    }

    log_message(app, "✗ Communication test failed");
    return FALSE;
}

void cleanup_devices(BridgeApp *app) {
    // Remove device files if they exist
    if (file_exists(app->device1_path)) {
        if (unlink(app->device1_path) == 0) {
            log_message(app, "Removed device: %s", app->device1_path);
        }
    }
    
    if (file_exists(app->device2_path)) {
        if (unlink(app->device2_path) == 0) {
            log_message(app, "Removed device: %s", app->device2_path);
        }
    }
}

gboolean set_device_permissions(BridgeApp *app) {
    if (!app->device_permissions) {
        return TRUE; // No specific permissions requested
    }

    mode_t mode = strtol(app->device_permissions, NULL, 8);
    
    if (chmod(app->device1_path, mode) != 0) {
        log_message(app, "WARNING: Could not set permissions for %s: %s", 
                   app->device1_path, strerror(errno));
        return FALSE;
    }
    
    if (chmod(app->device2_path, mode) != 0) {
        log_message(app, "WARNING: Could not set permissions for %s: %s", 
                   app->device2_path, strerror(errno));
        return FALSE;
    }

    log_message(app, "Set device permissions to %s", app->device_permissions);
    return TRUE;
}

void* monitor_socat_process(void *data) {
    BridgeApp *app = (BridgeApp *)data;
    
    while (app->monitor_running && app->running) {
        if (!is_null_modem_running(app)) {
            // Process died or devices disappeared
            g_idle_add((GSourceFunc)update_ui_state, app);
            break;
        }
        
        sleep(1); // Check every second
    }
    
    return NULL;
}
