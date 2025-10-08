#include "scripting.h"
#include "network.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Initialize Lua scripting engine
gboolean scripting_init(SerialTerminal *terminal) {
    if (!terminal) return FALSE;
    
    // Create new Lua state
    terminal->lua_state = luaL_newstate();
    if (!terminal->lua_state) {
        g_print("Error: Failed to create Lua state\n");
        return FALSE;
    }
    
    // Load standard Lua libraries
    luaL_openlibs(terminal->lua_state);
    
    // Register LAST API functions
    lua_register(terminal->lua_state, "log", lua_last_log);
    lua_register(terminal->lua_state, "send", lua_last_send);
    lua_register(terminal->lua_state, "get_connection_info", lua_last_get_connection_info);
    lua_register(terminal->lua_state, "get_statistics", lua_last_get_statistics);
    lua_register(terminal->lua_state, "parse_nmea", lua_last_parse_nmea);
    lua_register(terminal->lua_state, "create_nmea", lua_last_create_nmea);
    lua_register(terminal->lua_state, "calculate_checksum", lua_last_calculate_checksum);
    
    // Store terminal reference in Lua registry for API functions
    lua_pushlightuserdata(terminal->lua_state, terminal);
    lua_setfield(terminal->lua_state, LUA_REGISTRYINDEX, "terminal");
    
    terminal->scripting_enabled = FALSE;
    terminal->script_content = NULL;
    
    g_print("Lua scripting engine initialized successfully\n");
    return TRUE;
}

// Cleanup Lua scripting engine
void scripting_cleanup(SerialTerminal *terminal) {
    if (!terminal) return;
    
    if (terminal->lua_state) {
        lua_close(terminal->lua_state);
        terminal->lua_state = NULL;
    }
    
    if (terminal->script_content) {
        g_free(terminal->script_content);
        terminal->script_content = NULL;
    }
    
    terminal->scripting_enabled = FALSE;
}

// Load script content
gboolean scripting_load_script(SerialTerminal *terminal, const char *script_content) {
    if (!terminal || !terminal->lua_state || !script_content) return FALSE;
    
    // Clear existing script
    if (terminal->script_content) {
        g_free(terminal->script_content);
    }
    
    // Store new script content
    terminal->script_content = g_strdup(script_content);
    
    // Test script compilation
    int result = luaL_loadstring(terminal->lua_state, script_content);
    if (result != LUA_OK) {
        const char *error = lua_tostring(terminal->lua_state, -1);
        g_print("Script compilation error: %s\n", error);
        lua_pop(terminal->lua_state, 1);
        return FALSE;
    }
    
    // Pop the compiled function
    lua_pop(terminal->lua_state, 1);
    
    terminal->scripting_enabled = TRUE;
    g_print("Script loaded successfully\n");
    return TRUE;
}

// Load script from file
gboolean scripting_load_script_file(SerialTerminal *terminal, const char *filename) {
    if (!terminal || !filename) return FALSE;
    
    gchar *script_content = NULL;
    gsize length = 0;
    GError *error = NULL;
    
    if (!g_file_get_contents(filename, &script_content, &length, &error)) {
        g_print("Error loading script file: %s\n", error->message);
        g_error_free(error);
        return FALSE;
    }
    
    gboolean success = scripting_load_script(terminal, script_content);
    g_free(script_content);
    
    return success;
}

// Clear loaded script
void scripting_clear_script(SerialTerminal *terminal) {
    if (!terminal) return;
    
    if (terminal->script_content) {
        g_free(terminal->script_content);
        terminal->script_content = NULL;
    }
    
    terminal->scripting_enabled = FALSE;
    g_print("Script cleared\n");
}

// Execute script function with context
static ScriptResult* execute_script_function(SerialTerminal *terminal, const char *function_name, 
                                           const char *data, size_t length) {
    if (!terminal || !terminal->lua_state || !terminal->scripting_enabled || !function_name) {
        return NULL;
    }
    
    ScriptResult *result = g_malloc0(sizeof(ScriptResult));
    result->success = FALSE;
    result->suppress_original = FALSE;
    
    // Load the script
    int load_result = luaL_loadstring(terminal->lua_state, terminal->script_content);
    if (load_result != LUA_OK) {
        const char *error = lua_tostring(terminal->lua_state, -1);
        result->error_message = g_strdup(error);
        lua_pop(terminal->lua_state, 1);
        return result;
    }
    
    // Execute the script to define functions
    int exec_result = lua_pcall(terminal->lua_state, 0, 0, 0);
    if (exec_result != LUA_OK) {
        const char *error = lua_tostring(terminal->lua_state, -1);
        result->error_message = g_strdup(error);
        lua_pop(terminal->lua_state, 1);
        return result;
    }
    
    // Get the function
    lua_getglobal(terminal->lua_state, function_name);
    if (!lua_isfunction(terminal->lua_state, -1)) {
        lua_pop(terminal->lua_state, 1);
        result->success = TRUE; // Function doesn't exist, but that's OK
        return result;
    }
    
    // Push arguments
    if (data && length > 0) {
        lua_pushlstring(terminal->lua_state, data, length);
    } else {
        lua_pushnil(terminal->lua_state);
    }
    
    // Call the function
    int call_result = lua_pcall(terminal->lua_state, 1, 2, 0);
    if (call_result != LUA_OK) {
        const char *error = lua_tostring(terminal->lua_state, -1);
        result->error_message = g_strdup(error);
        lua_pop(terminal->lua_state, 1);
        return result;
    }
    
    // Get return values
    if (lua_isstring(terminal->lua_state, -2)) {
        size_t result_len;
        const char *result_data = lua_tolstring(terminal->lua_state, -2, &result_len);
        result->result_data = g_malloc(result_len + 1);
        memcpy(result->result_data, result_data, result_len);
        result->result_data[result_len] = '\0';
        result->result_length = result_len;
    }
    
    if (lua_isboolean(terminal->lua_state, -1)) {
        result->suppress_original = lua_toboolean(terminal->lua_state, -1);
    }
    
    lua_pop(terminal->lua_state, 2);
    result->success = TRUE;
    
    return result;
}

// Execute script on data received
ScriptResult* scripting_execute_on_data_received(SerialTerminal *terminal, const char *data, size_t length) {
    return execute_script_function(terminal, "on_data_received", data, length);
}

// Execute script on data send
ScriptResult* scripting_execute_on_data_send(SerialTerminal *terminal, const char *data, size_t length) {
    return execute_script_function(terminal, "on_data_send", data, length);
}

// Execute script on connection open
ScriptResult* scripting_execute_on_connection_open(SerialTerminal *terminal) {
    return execute_script_function(terminal, "on_connection_open", NULL, 0);
}

// Execute script on connection close
ScriptResult* scripting_execute_on_connection_close(SerialTerminal *terminal) {
    return execute_script_function(terminal, "on_connection_close", NULL, 0);
}

// Execute manual script code
ScriptResult* scripting_execute_manual(SerialTerminal *terminal, const char *script_code) {
    if (!terminal || !terminal->lua_state || !script_code) return NULL;
    
    ScriptResult *result = g_malloc0(sizeof(ScriptResult));
    result->success = FALSE;
    
    int exec_result = luaL_dostring(terminal->lua_state, script_code);
    if (exec_result != LUA_OK) {
        const char *error = lua_tostring(terminal->lua_state, -1);
        result->error_message = g_strdup(error);
        lua_pop(terminal->lua_state, 1);
        return result;
    }
    
    result->success = TRUE;
    return result;
}

// Free script result
void scripting_free_result(ScriptResult *result) {
    if (!result) return;
    
    if (result->result_data) {
        g_free(result->result_data);
    }
    if (result->error_message) {
        g_free(result->error_message);
    }
    g_free(result);
}

// Convert script context to string
const char* script_context_to_string(ScriptContext context) {
    switch (context) {
        case SCRIPT_CONTEXT_DATA_RECEIVED: return "Data Received";
        case SCRIPT_CONTEXT_DATA_SEND: return "Data Send";
        case SCRIPT_CONTEXT_CONNECTION_OPEN: return "Connection Open";
        case SCRIPT_CONTEXT_CONNECTION_CLOSE: return "Connection Close";
        case SCRIPT_CONTEXT_MANUAL: return "Manual";
        default: return "Unknown";
    }
}

// Validate Lua script syntax
gboolean is_valid_lua_script(const char *script_content) {
    if (!script_content) return FALSE;

    lua_State *test_state = luaL_newstate();
    if (!test_state) return FALSE;

    int result = luaL_loadstring(test_state, script_content);
    gboolean valid = (result == LUA_OK);

    lua_close(test_state);
    return valid;
}

// Lua API Functions (exposed to scripts)

// log(message) - Add message to terminal log
int lua_last_log(lua_State *L) {
    const char *message = luaL_checkstring(L, 1);
    g_print("Script: %s\n", message);
    return 0;
}

// send(data) - Send data through current connection
int lua_last_send(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "terminal");
    SerialTerminal *terminal = (SerialTerminal*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (!terminal) {
        lua_pushboolean(L, 0);
        return 1;
    }

    size_t length;
    const char *data = luaL_checklstring(L, 1, &length);

    ssize_t sent = 0;
    if (terminal->connection_type == CONNECTION_TYPE_SERIAL) {
        sent = write(terminal->connection_fd, data, length);
    } else {
        sent = network_send_data(terminal, data, length);
    }

    lua_pushboolean(L, sent > 0);
    return 1;
}

// get_connection_info() - Get connection details
int lua_last_get_connection_info(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "terminal");
    SerialTerminal *terminal = (SerialTerminal*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (!terminal) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    // Connection type
    lua_pushstring(L, connection_type_to_string(terminal->connection_type));
    lua_setfield(L, -2, "type");

    // Connection status
    lua_pushboolean(L, terminal->connection_fd >= 0);
    lua_setfield(L, -2, "connected");

    if (terminal->connection_type == CONNECTION_TYPE_SERIAL) {
        // Serial-specific info
        const char *port = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->port_combo));
        if (port) {
            lua_pushstring(L, port);
            lua_setfield(L, -2, "port");
        }

        const char *baud = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(terminal->baudrate_combo));
        if (baud) {
            lua_pushstring(L, baud);
            lua_setfield(L, -2, "baud_rate");
        }
    } else {
        // Network-specific info
        lua_pushstring(L, terminal->network_host);
        lua_setfield(L, -2, "host");

        lua_pushstring(L, terminal->network_port);
        lua_setfield(L, -2, "port");
    }

    return 1;
}

// get_statistics() - Get connection statistics
int lua_last_get_statistics(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "terminal");
    SerialTerminal *terminal = (SerialTerminal*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (!terminal) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    lua_pushinteger(L, terminal->bytes_sent);
    lua_setfield(L, -2, "bytes_sent");

    lua_pushinteger(L, terminal->bytes_received);
    lua_setfield(L, -2, "bytes_received");

    if (terminal->connection_start_time > 0) {
        time_t current_time = time(NULL);
        lua_pushinteger(L, current_time - terminal->connection_start_time);
        lua_setfield(L, -2, "connection_duration");
    }

    return 1;
}

// calculate_checksum(data) - Calculate NMEA checksum
int lua_last_calculate_checksum(lua_State *L) {
    size_t length;
    const char *data = luaL_checklstring(L, 1, &length);

    unsigned char checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum ^= (unsigned char)data[i];
    }

    char checksum_str[3];
    snprintf(checksum_str, sizeof(checksum_str), "%02X", checksum);

    lua_pushstring(L, checksum_str);
    return 1;
}

// parse_nmea(sentence) - Parse NMEA sentence
int lua_last_parse_nmea(lua_State *L) {
    const char *sentence = luaL_checkstring(L, 1);

    if (!sentence || sentence[0] != '$') {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    // Find the end of the sentence (before checksum)
    const char *checksum_start = strrchr(sentence, '*');
    size_t data_length = checksum_start ? (checksum_start - sentence - 1) : (strlen(sentence) - 1);

    // Extract talker ID (first 2 characters after $)
    if (data_length >= 2) {
        char talker[3] = {sentence[1], sentence[2], '\0'};
        lua_pushstring(L, talker);
        lua_setfield(L, -2, "talker");
    }

    // Extract sentence type (next 3 characters)
    if (data_length >= 5) {
        char sentence_type[4] = {sentence[3], sentence[4], sentence[5], '\0'};
        lua_pushstring(L, sentence_type);
        lua_setfield(L, -2, "type");
    }

    // Extract data fields (comma-separated)
    if (data_length > 6) {
        const char *data_start = sentence + 6;
        char *data_copy = g_strndup(data_start, data_length - 5);

        lua_newtable(L);
        char *token = strtok(data_copy, ",");
        int field_index = 1;

        while (token != NULL) {
            lua_pushstring(L, token);
            lua_rawseti(L, -2, field_index++);
            token = strtok(NULL, ",");
        }

        lua_setfield(L, -2, "fields");
        g_free(data_copy);
    }

    // Extract checksum if present
    if (checksum_start && strlen(checksum_start) >= 3) {
        char checksum[3] = {checksum_start[1], checksum_start[2], '\0'};
        lua_pushstring(L, checksum);
        lua_setfield(L, -2, "checksum");
    }

    return 1;
}

// create_nmea(talker, sentence_type, fields) - Create NMEA sentence
int lua_last_create_nmea(lua_State *L) {
    const char *talker = luaL_checkstring(L, 1);
    const char *sentence_type = luaL_checkstring(L, 2);

    if (!lua_istable(L, 3)) {
        lua_pushnil(L);
        return 1;
    }

    // Start building the sentence
    GString *sentence = g_string_new("$");
    g_string_append(sentence, talker);
    g_string_append(sentence, sentence_type);

    // Add fields from table
    lua_pushnil(L);
    while (lua_next(L, 3) != 0) {
        g_string_append_c(sentence, ',');
        if (lua_isstring(L, -1)) {
            g_string_append(sentence, lua_tostring(L, -1));
        } else if (lua_isnumber(L, -1)) {
            g_string_append_printf(sentence, "%.6f", lua_tonumber(L, -1));
        }
        lua_pop(L, 1);
    }

    // Calculate checksum
    unsigned char checksum = 0;
    for (size_t i = 1; i < sentence->len; i++) {
        checksum ^= (unsigned char)sentence->str[i];
    }

    // Add checksum
    g_string_append_printf(sentence, "*%02X", checksum);

    lua_pushstring(L, sentence->str);
    g_string_free(sentence, TRUE);

    return 1;
}
