/*
 * Null modem module header for BRIDGE - Virtual Null Modem Bridge
 * Handles socat process management and virtual device operations
 */

#ifndef NULLMODEM_H
#define NULLMODEM_H

#include "common.h"

// Function declarations
gboolean create_null_modem(BridgeApp *app);
void stop_null_modem(BridgeApp *app);
gboolean is_null_modem_running(BridgeApp *app);
gboolean test_null_modem_communication(BridgeApp *app);
void cleanup_devices(BridgeApp *app);
gboolean check_socat_available(void);
void* monitor_socat_process(void *data);
gboolean set_device_permissions(BridgeApp *app);

#endif // NULLMODEM_H
