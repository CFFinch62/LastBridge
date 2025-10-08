# LAST Lua Scripting Engine Guide

## 📖 Table of Contents
1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Event Handlers](#event-handlers)
4. [API Functions](#api-functions)
5. [Marine Electronics Examples](#marine-electronics-examples)
6. [Advanced Techniques](#advanced-techniques)
7. [Troubleshooting](#troubleshooting)

## 🚀 Introduction

The LAST Lua Scripting Engine provides powerful real-time data processing capabilities for serial and network communications. Built on Lua 5.4.7, it offers:

- **Event-driven processing** - Scripts execute automatically on data events
- **Marine electronics focus** - Built-in NMEA parsing and creation
- **Real-time execution** - Process data streams without blocking communication
- **Zero dependencies** - Lua engine embedded directly in LAST
- **Separate interface** - Dedicated script window for development and testing

## 🏁 Getting Started

### Opening the Script Window
1. Launch LAST application
2. Go to **Tools** → **Lua Scripting...**
3. The script window opens with a default template

### Basic Script Structure
```lua
-- LAST Lua Script Template
-- This script demonstrates all available event handlers

function on_data_received(data)
    -- Process incoming data
    -- data: string containing received data
    -- Return: modified_data, suppress_original
    log("Received: " .. data)
    return nil, false  -- Don't modify, don't suppress
end

function on_data_send(data)
    -- Process outgoing data before transmission
    -- data: string containing data to be sent
    -- Return: modified_data, suppress_original
    log("Sending: " .. data)
    return nil, false  -- Don't modify, don't suppress
end

function on_connection_open()
    -- Execute when connection is established
    log("Connection opened!")
end

function on_connection_close()
    -- Execute when connection is closed
    log("Connection closed!")
end
```

### Script Management
- **Enable/Disable**: Check/uncheck "Enable Scripting"
- **Load Script**: Load .lua files from disk
- **Save Script**: Save current script to .lua file
- **Test Script**: Validate syntax and test execution
- **Clear Script**: Reset to default template

## 🎯 Event Handlers

### on_data_received(data)
**Purpose**: Process incoming data from serial port or network connection

**Parameters**:
- `data` (string): Raw data received from connection

**Return Values**:
- `modified_data` (string or nil): Replacement data to display
- `suppress_original` (boolean): If true, original data won't be displayed

**Examples**:
```lua
-- Log all received data
function on_data_received(data)
    log("RX: " .. data)
    return nil, false
end

-- Filter out debug messages
function on_data_received(data)
    if string.find(data, "DEBUG") then
        return nil, true  -- Suppress debug messages
    end
    return nil, false
end

-- Transform data before display
function on_data_received(data)
    local clean_data = string.gsub(data, "\r\n", "\n")
    return clean_data, false
end
```

### on_data_send(data)
**Purpose**: Process outgoing data before transmission

**Parameters**:
- `data` (string): Data about to be sent

**Return Values**:
- `modified_data` (string or nil): Replacement data to send
- `suppress_original` (boolean): If true, original data won't be sent

**Examples**:
```lua
-- Add timestamp to outgoing data
function on_data_send(data)
    local timestamp = os.date("%H:%M:%S")
    local timestamped = "[" .. timestamp .. "] " .. data
    return timestamped, false
end

-- Auto-add NMEA checksum
function on_data_send(data)
    if string.sub(data, 1, 1) == "$" and not string.find(data, "*") then
        local checksum = calculate_checksum(data)
        local with_checksum = data .. "*" .. checksum
        return with_checksum, false
    end
    return nil, false
end

-- Block certain commands
function on_data_send(data)
    if string.find(data, "DANGEROUS_COMMAND") then
        log("Blocked dangerous command!")
        return nil, true  -- Suppress sending
    end
    return nil, false
end
```

### on_connection_open()
**Purpose**: Execute when connection is established

**Examples**:
```lua
function on_connection_open()
    local info = get_connection_info()
    log("Connected to " .. info.type .. " on " .. (info.port or info.host))
    
    -- Send initialization sequence
    if info.type == "TCP Client" then
        send("$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28")  -- GPS setup
    end
end
```

### on_connection_close()
**Purpose**: Execute when connection is closed

**Examples**:
```lua
function on_connection_close()
    local stats = get_statistics()
    log("Connection closed. Sent: " .. stats.bytes_sent .. " bytes, Received: " .. stats.bytes_received .. " bytes")
end
```

## 🔧 API Functions

### log(message)
**Purpose**: Add messages to the terminal log

**Parameters**:
- `message` (string): Message to display in terminal

**Example**:
```lua
log("Script started successfully")
log("Processing NMEA data...")
```

### send(data)
**Purpose**: Send data through current connection

**Parameters**:
- `data` (string): Data to transmit

**Example**:
```lua
-- Send GPS configuration
send("$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28")

-- Send autopilot command
send("$STALK,84,06,00,00*48")
```

### get_connection_info()
**Purpose**: Get current connection details

**Returns**: Table with connection information
- `type` (string): "Serial", "TCP Client", "TCP Server", "UDP Client", "UDP Server"
- `port` (string): Serial port or network port
- `host` (string): Network hostname/IP (network connections only)
- `baud` (string): Baud rate (serial connections only)

**Example**:
```lua
function on_connection_open()
    local info = get_connection_info()
    if info.type == "Serial" then
        log("Serial connection on " .. info.port .. " at " .. info.baud .. " baud")
    else
        log("Network connection: " .. info.type .. " to " .. info.host .. ":" .. info.port)
    end
end
```

### get_statistics()
**Purpose**: Get connection statistics

**Returns**: Table with statistics
- `bytes_sent` (number): Total bytes transmitted
- `bytes_received` (number): Total bytes received
- `connection_time` (number): Connection duration in seconds

**Example**:
```lua
function show_stats()
    local stats = get_statistics()
    log("Statistics: TX=" .. stats.bytes_sent .. " RX=" .. stats.bytes_received .. 
        " Time=" .. stats.connection_time .. "s")
end
```

### calculate_checksum(data)
**Purpose**: Calculate NMEA checksum for data

**Parameters**:
- `data` (string): NMEA sentence (without checksum)

**Returns**: String containing hexadecimal checksum

**Example**:
```lua
local sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*"
local checksum = calculate_checksum(sentence)
log("Checksum: " .. checksum)  -- Output: "47"
```

### parse_nmea(sentence)
**Purpose**: Parse NMEA sentence into structured data

**Parameters**:
- `sentence` (string): Complete NMEA sentence

**Returns**: Table with parsed data or nil if invalid
- `talker` (string): Talker ID (e.g., "GP", "GL", "GN")
- `type` (string): Message type (e.g., "GGA", "RMC", "VTG")
- `fields` (table): Array of data fields
- `checksum` (string): Checksum from sentence
- `valid` (boolean): True if checksum is valid

**Example**:
```lua
function on_data_received(data)
    local nmea = parse_nmea(data)
    if nmea and nmea.valid then
        if nmea.type == "GGA" then
            log("GPS Fix: Lat=" .. (nmea.fields[2] or "unknown") .. 
                " Lon=" .. (nmea.fields[4] or "unknown"))
        elseif nmea.type == "RMC" then
            log("GPS Speed: " .. (nmea.fields[7] or "unknown") .. " knots")
        end
    end
    return nil, false
end
```

### create_nmea(talker, type, fields)
**Purpose**: Create NMEA sentence with automatic checksum

**Parameters**:
- `talker` (string): Talker ID (e.g., "GP", "GL")
- `type` (string): Message type (e.g., "GGA", "RMC")
- `fields` (table): Array of field values

**Returns**: Complete NMEA sentence with checksum

**Example**:
```lua
-- Create custom GPS sentence
local fields = {"123519", "4807.038", "N", "01131.000", "E", "1", "08", "0.9", "545.4", "M", "46.9", "M", "", ""}
local sentence = create_nmea("GP", "GGA", fields)
send(sentence)  -- Sends: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
```

## 🌊 Marine Electronics Examples

### GPS Data Processing
```lua
-- Monitor GPS position and log significant changes
local last_lat, last_lon = nil, nil

function on_data_received(data)
    local nmea = parse_nmea(data)
    if nmea and nmea.valid and nmea.type == "GGA" then
        local lat = nmea.fields[2]
        local lon = nmea.fields[4]

        if lat and lon and (lat ~= last_lat or lon ~= last_lon) then
            log("Position update: " .. lat .. " " .. (nmea.fields[3] or "") ..
                ", " .. lon .. " " .. (nmea.fields[5] or ""))
            last_lat, last_lon = lat, lon
        end
    end
    return nil, false
end
```

### Autopilot Integration
```lua
-- Automatic autopilot responses
function on_data_received(data)
    -- Respond to autopilot status queries
    if string.find(data, "!Q") then
        send("!AP,1")  -- Start autopilot
        log("Autopilot started automatically")
    elseif string.find(data, "!ALARM") then
        send("!AP,0")  -- Stop autopilot on alarm
        log("Autopilot stopped due to alarm")
    end
    return nil, false
end
```

### Fish Finder Data Logging
```lua
-- Log depth readings from fish finder
function on_data_received(data)
    local nmea = parse_nmea(data)
    if nmea and nmea.valid and nmea.type == "DBT" then
        local depth = nmea.fields[3]  -- Depth in meters
        if depth and tonumber(depth) then
            log("Depth: " .. depth .. "m")

            -- Alert on shallow water
            if tonumber(depth) < 2.0 then
                log("WARNING: Shallow water detected!")
                send("!ALARM,SHALLOW")
            end
        end
    end
    return nil, false
end
```

### AIS Traffic Monitoring
```lua
-- Monitor AIS vessel traffic
local vessels = {}

function on_data_received(data)
    local nmea = parse_nmea(data)
    if nmea and nmea.valid and (nmea.type == "VDM" or nmea.type == "VDO") then
        -- AIS message received
        local mmsi = extract_mmsi(nmea.fields[5])  -- Simplified extraction
        if mmsi then
            vessels[mmsi] = os.time()
            log("AIS vessel " .. mmsi .. " detected")
        end
    end
    return nil, false
end

function extract_mmsi(payload)
    -- Simplified MMSI extraction (real implementation would decode AIS payload)
    if payload and string.len(payload) > 10 then
        return "123456789"  -- Placeholder
    end
    return nil
end
```

### Multi-Device Communication
```lua
-- Route data between multiple devices
local device_ports = {
    gps = "10110",
    autopilot = "10111",
    fishfinder = "10112"
}

function on_data_received(data)
    local nmea = parse_nmea(data)
    if nmea and nmea.valid then
        -- Route GPS data to autopilot
        if nmea.type == "RMC" or nmea.type == "GGA" then
            -- Would need multiple connections - this is conceptual
            log("Routing GPS data to autopilot")
        end

        -- Route depth data to chart plotter
        if nmea.type == "DBT" or nmea.type == "DPT" then
            log("Routing depth data to chart plotter")
        end
    end
    return nil, false
end
```

### Data Validation and Correction
```lua
-- Validate and correct NMEA data
function on_data_received(data)
    local nmea = parse_nmea(data)
    if nmea then
        if not nmea.valid then
            -- Attempt to recalculate checksum
            local corrected = string.gsub(data, "%*%w%w", "")
            local checksum = calculate_checksum(corrected)
            local fixed = corrected .. "*" .. checksum
            log("Corrected invalid checksum: " .. data .. " -> " .. fixed)
            return fixed, false
        end

        -- Validate GPS coordinates
        if nmea.type == "GGA" then
            local lat = tonumber(nmea.fields[2])
            local lon = tonumber(nmea.fields[4])
            if lat and (lat < 0 or lat > 90) then
                log("WARNING: Invalid latitude: " .. lat)
                return nil, true  -- Suppress invalid data
            end
            if lon and (lon < 0 or lon > 180) then
                log("WARNING: Invalid longitude: " .. lon)
                return nil, true  -- Suppress invalid data
            end
        end
    end
    return nil, false
end
```

## 🎓 Advanced Techniques

### State Management
```lua
-- Maintain state between script executions
local script_state = {
    connection_count = 0,
    last_position = nil,
    device_status = {},
    message_history = {}
}

function on_connection_open()
    script_state.connection_count = script_state.connection_count + 1
    log("Connection #" .. script_state.connection_count .. " established")
end

function on_data_received(data)
    -- Store message history
    table.insert(script_state.message_history, {
        timestamp = os.time(),
        data = data
    })

    -- Keep only last 100 messages
    if #script_state.message_history > 100 then
        table.remove(script_state.message_history, 1)
    end

    return nil, false
end
```

### Conditional Processing
```lua
-- Process data differently based on connection type
function on_data_received(data)
    local info = get_connection_info()

    if info.type == "Serial" then
        -- Serial-specific processing
        log("Serial data: " .. data)
    elseif info.type == "TCP Client" then
        -- Network-specific processing
        local nmea = parse_nmea(data)
        if nmea and nmea.valid then
            log("Network NMEA: " .. nmea.type)
        end
    end

    return nil, false
end
```

### Data Buffering
```lua
-- Buffer partial messages until complete
local message_buffer = ""

function on_data_received(data)
    message_buffer = message_buffer .. data

    -- Process complete lines
    while string.find(message_buffer, "\n") do
        local line, rest = string.match(message_buffer, "([^\n]*)\n(.*)")
        if line then
            process_complete_message(line)
            message_buffer = rest or ""
        else
            break
        end
    end

    return nil, false
end

function process_complete_message(message)
    local nmea = parse_nmea(message)
    if nmea and nmea.valid then
        log("Complete message: " .. nmea.type)
    end
end
```

### Timing and Scheduling
```lua
-- Send periodic status requests
local last_status_request = 0
local status_interval = 30  -- seconds

function on_data_received(data)
    local current_time = os.time()

    -- Send status request every 30 seconds
    if current_time - last_status_request >= status_interval then
        send("!Q")  -- Query device status
        last_status_request = current_time
        log("Status request sent")
    end

    return nil, false
end
```

## 🔍 Troubleshooting

### Common Issues

**Script Not Executing**
- Ensure "Enable Scripting" is checked
- Check for syntax errors using "Test Script" button
- Verify function names are spelled correctly

**Syntax Errors**
```lua
-- Common mistakes:
-- Missing 'end' keyword
function on_data_received(data)
    log("test")
-- end  -- Don't forget this!

-- Incorrect return values
function on_data_received(data)
    return "modified"  -- Missing second parameter
    -- Should be: return "modified", false
end
```

**Performance Issues**
- Avoid complex processing in event handlers
- Use efficient string operations
- Limit log output in high-frequency events

**Memory Usage**
- Clear large tables periodically
- Avoid storing unlimited history
- Use local variables when possible

### Debugging Techniques

**Add Debug Logging**
```lua
function on_data_received(data)
    log("DEBUG: Received " .. string.len(data) .. " bytes")
    log("DEBUG: Data content: " .. data)

    local nmea = parse_nmea(data)
    if nmea then
        log("DEBUG: Parsed NMEA type: " .. (nmea.type or "unknown"))
    else
        log("DEBUG: Not valid NMEA data")
    end

    return nil, false
end
```

**Test Individual Functions**
```lua
-- Test NMEA parsing
function test_nmea_parsing()
    local test_sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"
    local result = parse_nmea(test_sentence)
    if result then
        log("Test passed: " .. result.type)
    else
        log("Test failed: Could not parse NMEA")
    end
end

-- Call test function manually
test_nmea_parsing()
```

### Best Practices

1. **Keep Scripts Simple** - Complex logic can impact performance
2. **Handle Errors Gracefully** - Always check for nil values
3. **Use Descriptive Logging** - Help with debugging and monitoring
4. **Test Thoroughly** - Use "Test Script" before enabling
5. **Document Your Code** - Add comments for complex logic
6. **Version Control** - Save different script versions as separate files

### Script Templates

**Basic Data Logger**
```lua
function on_data_received(data)
    local timestamp = os.date("%Y-%m-%d %H:%M:%S")
    log("[" .. timestamp .. "] RX: " .. data)
    return nil, false
end

function on_data_send(data)
    local timestamp = os.date("%Y-%m-%d %H:%M:%S")
    log("[" .. timestamp .. "] TX: " .. data)
    return nil, false
end
```

**NMEA Monitor**
```lua
function on_data_received(data)
    local nmea = parse_nmea(data)
    if nmea and nmea.valid then
        log("NMEA " .. nmea.talker .. nmea.type .. " - " .. #nmea.fields .. " fields")
    end
    return nil, false
end
```

**Connection Monitor**
```lua
function on_connection_open()
    local info = get_connection_info()
    log("Connected: " .. info.type .. " (" .. (info.port or info.host) .. ")")
end

function on_connection_close()
    local stats = get_statistics()
    log("Disconnected - TX:" .. stats.bytes_sent .. " RX:" .. stats.bytes_received)
end
```

---

## 📚 Additional Resources

- **Lua 5.4 Reference Manual**: https://www.lua.org/manual/5.4/
- **NMEA 0183 Standard**: For marine electronics protocol details
- **LAST GitHub Repository**: For updates and examples

For more examples and community scripts, visit the LAST project repository.
