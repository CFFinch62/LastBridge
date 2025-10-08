-- Example Lua Script for LAST Serial Terminal
-- This script demonstrates the available functions and event handlers

-- Global variables for script state
local message_count = 0
local connection_time = 0

-- Called when data is received from the serial connection
function on_data_received(data)
    message_count = message_count + 1
    
    -- Log received data with count
    log("Received message #" .. message_count .. ": " .. data)
    
    -- Check if it's an NMEA sentence
    if string.sub(data, 1, 1) == "$" then
        local parsed = parse_nmea(data)
        if parsed then
            log("NMEA sentence parsed: " .. parsed.talker .. parsed.sentence)
        end
    end
    
    -- Auto-respond to specific commands
    if string.find(data, "PING") then
        send("PONG\r\n")
        log("Auto-responded with PONG")
    end
    
    -- Return nil to allow normal processing
    return nil
end

-- Called before data is sent through the connection
function on_data_send(data)
    log("Sending: " .. data)
    
    -- You could modify the data here before sending
    -- For example, add a timestamp or checksum
    
    -- Return nil to send original data, or return modified data
    return nil
end

-- Called when connection is established
function on_connection_open()
    connection_time = os.time()
    log("Connection opened at " .. os.date("%Y-%m-%d %H:%M:%S", connection_time))
    
    -- Get connection info
    local info = get_connection_info()
    if info then
        log("Connection details: " .. info.type .. " - " .. info.details)
    end
    
    -- Send a welcome message after 1 second (simulated)
    -- Note: In a real implementation, you might want to use a timer
    -- send("Welcome! Lua scripting is active.\r\n")
end

-- Called when connection is closed
function on_connection_close()
    local duration = os.time() - connection_time
    log("Connection closed after " .. duration .. " seconds")
    log("Total messages processed: " .. message_count)
    
    -- Reset counters
    message_count = 0
    connection_time = 0
end

-- Example function to create an NMEA sentence
function create_test_nmea()
    local nmea = create_nmea("GP", "GGA", "123456.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,")
    if nmea then
        log("Created NMEA sentence: " .. nmea)
        return nmea
    end
    return nil
end

-- Example function to demonstrate statistics
function show_stats()
    local stats = get_statistics()
    if stats then
        log("Statistics - Sent: " .. stats.bytes_sent .. ", Received: " .. stats.bytes_received)
    end
end

-- Utility function for logging with timestamp
function log_with_time(message)
    log("[" .. os.date("%H:%M:%S") .. "] " .. message)
end

log("Lua script loaded successfully!")
