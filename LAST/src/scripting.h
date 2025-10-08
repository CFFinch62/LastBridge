#ifndef SCRIPTING_H
#define SCRIPTING_H

#include "common.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// Script execution contexts
typedef enum {
    SCRIPT_CONTEXT_DATA_RECEIVED = 0,
    SCRIPT_CONTEXT_DATA_SEND,
    SCRIPT_CONTEXT_CONNECTION_OPEN,
    SCRIPT_CONTEXT_CONNECTION_CLOSE,
    SCRIPT_CONTEXT_MANUAL
} ScriptContext;

// Script execution result
typedef struct {
    gboolean success;
    char *result_data;      // Modified/generated data (if any)
    size_t result_length;   // Length of result data
    char *error_message;    // Error message (if failed)
    gboolean suppress_original; // If true, don't send/display original data
} ScriptResult;

// Script management functions
gboolean scripting_init(SerialTerminal *terminal);
void scripting_cleanup(SerialTerminal *terminal);
gboolean scripting_load_script(SerialTerminal *terminal, const char *script_content);
gboolean scripting_load_script_file(SerialTerminal *terminal, const char *filename);
void scripting_clear_script(SerialTerminal *terminal);

// Script execution functions
ScriptResult* scripting_execute_on_data_received(SerialTerminal *terminal, const char *data, size_t length);
ScriptResult* scripting_execute_on_data_send(SerialTerminal *terminal, const char *data, size_t length);
ScriptResult* scripting_execute_on_connection_open(SerialTerminal *terminal);
ScriptResult* scripting_execute_on_connection_close(SerialTerminal *terminal);
ScriptResult* scripting_execute_manual(SerialTerminal *terminal, const char *script_code);

// Script result management
void scripting_free_result(ScriptResult *result);

// Lua API functions (exposed to scripts)
int lua_last_log(lua_State *L);           // log(message) - add to terminal log
int lua_last_send(lua_State *L);          // send(data) - send data through connection
int lua_last_get_connection_info(lua_State *L); // get_connection_info() - get connection details
int lua_last_get_statistics(lua_State *L); // get_statistics() - get connection stats
int lua_last_parse_nmea(lua_State *L);    // parse_nmea(sentence) - parse NMEA sentence
int lua_last_create_nmea(lua_State *L);   // create_nmea(talker, sentence, data) - create NMEA sentence
int lua_last_calculate_checksum(lua_State *L); // calculate_checksum(data) - calculate NMEA checksum

// Utility functions
const char* script_context_to_string(ScriptContext context);
gboolean is_valid_lua_script(const char *script_content);

#endif // SCRIPTING_H
