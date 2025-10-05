# BRIDGE Serial Data Sniffing Guide

## Overview

The enhanced BRIDGE application now includes comprehensive serial data sniffing capabilities, transforming it into a universal serial communication analyzer while maintaining full backward compatibility with the original null modem functionality.

## Key Features

- **Universal Serial Sniffer**: Capture and analyze any serial communication
- **Multiple Output Methods**: Named pipes, TCP sockets, UDP streams, and file logging
- **Real-time Data Streaming**: Live data feeds for external applications
- **Configurable Filtering**: RX only, TX only, or bidirectional capture
- **Multiple Data Formats**: Raw binary, hex dump, or formatted text
- **Professional Grade**: Comparable to expensive hardware serial analyzers
- **Thread-safe Implementation**: Robust and reliable operation
- **Laptop-Friendly GUI**: Optimized landscape layout for smaller screens

## Architecture

```
Device A ←→ /tmp/ttyDevice1 ←→ Enhanced BRIDGE ←→ /tmp/ttyDevice2 ←→ Device B
                                        ↓
                               Data Sniffing Layer
                                        ↓
                    ┌─────────────────────────────────┐
                    │  Multiple Output Streams        │
                    ├─────────────────────────────────┤
                    │ • Named Pipe: /tmp/bridge_sniff │
                    │ • TCP Socket: localhost:8888    │
                    │ • UDP Stream: 239.1.1.1:9999   │
                    │ • File Output: bridge_data.log  │
                    └─────────────────────────────────┘
                                        ↓
                          External Processing Applications
```

## Quick Start Guide

### 1. Basic Setup

1. **Start BRIDGE**: Launch the enhanced BRIDGE application (optimized for laptop screens)
2. **Configure Devices**: Set up your virtual device paths (e.g., `/tmp/ttyBBFF` and `/tmp/ttyLAST`)
3. **Enable Sniffing**: Go to the "Sniffing" tab and check "Enable Sniffing"
4. **Select Output Methods**: Choose your desired output methods in the left column (TCP recommended)
5. **Start Bridge**: Click "Start Virtual Null Modem" to create the devices
6. **Start Sniffing**: Click "Start" to begin data capture

### 2. Connect Your Applications

- **Device A** connects to `/tmp/ttyBBFF`
- **Device B** (like LAST) connects to `/tmp/ttyLAST`
- **Your analysis application** connects to the sniffing output (e.g., TCP port 8888)

### 3. Monitor Data Flow

All serial communication between Device A and Device B will be:
- Passed through transparently (normal operation continues)
- Simultaneously streamed to your selected outputs
- Available for real-time analysis and processing

## Output Methods

### Named Pipe
- **Path**: `/tmp/bridge_sniff_pipe` (configurable)
- **Use Case**: Local applications on the same system
- **Example**: `cat /tmp/bridge_sniff_pipe`

### TCP Socket
- **Port**: `8888` (configurable)
- **Use Case**: Network applications, multiple clients
- **Example**: `nc localhost 8888`
- **Supports**: Multiple simultaneous connections

### UDP Stream
- **Address**: `239.1.1.1:9999` (configurable)
- **Use Case**: Broadcast to multiple listeners, real-time streaming
- **Example**: Custom UDP client applications

### File Logging
- **Path**: Auto-generated or custom filename
- **Use Case**: Session recording, offline analysis
- **Format**: Timestamped entries with direction indicators

## Data Formats

### Raw Binary
- Unmodified data as received/transmitted
- Best for: Binary protocols, exact data reproduction

### Hex Dump
- Human-readable hexadecimal representation
- Format: `HH:MM:SS R: 48 65 6C 6C 6F 0D 0A`
- Best for: Protocol analysis, debugging

### Text
- ASCII representation with timestamps
- Format: `HH:MM:SS R: Hello\r\n`
- Best for: Text-based protocols, human reading

## Filtering Options

### Data Direction
- **Both RX & TX**: Capture all communication (default)
- **RX Only**: Capture only received data
- **TX Only**: Capture only transmitted data

### Use Cases
- **Protocol Analysis**: Use "Both" to see complete conversation
- **Response Monitoring**: Use "RX Only" to focus on device responses
- **Command Monitoring**: Use "TX Only" to focus on commands sent

## Example Use Cases

### 1. BBFF Video Data Processing
```
BBFF Module ←→ /tmp/ttyBBFF ←→ BRIDGE ←→ /tmp/ttyLAST ←→ LAST (Control)
                                  ↓
                          TCP:8888 → Video Processing App
```

### 2. GPS Protocol Analysis
```
GPS Unit ←→ /tmp/ttyGPS ←→ BRIDGE ←→ /tmp/ttyNav ←→ Navigation Software
                              ↓
                      File → NMEA Analysis Log
```

### 3. Device Reverse Engineering
```
Unknown Device ←→ /tmp/ttyDev ←→ BRIDGE ←→ /tmp/ttyPC ←→ PC Software
                                    ↓
                            TCP:8888 → Protocol Analyzer
```

## Example Client Applications

### TCP Client (Python)
```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 8888))

while True:
    data = sock.recv(1024)
    if not data:
        break
    print(f"Received: {data.decode('utf-8', errors='ignore')}")
```

### Named Pipe Reader (Bash)
```bash
#!/bin/bash
while IFS= read -r line; do
    echo "Sniffed: $line"
done < /tmp/bridge_sniff_pipe
```

### C Client Example
See `examples/sniff_client.c` for a complete C implementation supporting all output methods.

## Integration with LAST

The enhanced BRIDGE works seamlessly with LAST:

1. **Start BRIDGE** with sniffing enabled
2. **Configure devices** (e.g., `/tmp/ttyBBFF` and `/tmp/ttyLAST`)
3. **Start the bridge** and sniffing
4. **Launch LAST** and connect to `/tmp/ttyLAST`
5. **Connect your device** to `/tmp/ttyBBFF`
6. **Use LAST normally** for control and monitoring
7. **Your sniffing client** receives all data in real-time

## GUI Design

The BRIDGE sniffing interface is optimized for laptop screens and smaller displays:

### Layout Features
- **Landscape-Oriented**: Horizontal two-column layout maximizes screen usage
- **Compact Controls**: Efficient spacing and shorter labels reduce vertical space
- **Organized Sections**:
  - **Left Column**: Output methods and configuration options
  - **Right Column**: Statistics and quick reference guide
- **Laptop-Friendly**: Default window size (800x400) fits comfortably on most screens

### User Interface Tips
- All sniffing controls are accessible without scrolling
- Output method checkboxes are clearly grouped with their configuration fields
- Quick guide provides essential connection information at a glance
- Statistics update in real-time during sniffing operations

## Performance Considerations

- **Minimal Overhead**: Sniffing adds negligible latency to serial communication
- **Thread-safe**: Sniffing runs in separate threads without blocking main communication
- **Configurable Buffering**: Adjust buffer sizes for high-throughput applications
- **Multiple Clients**: TCP output supports multiple simultaneous connections

## Troubleshooting

### Common Issues

1. **"Permission denied" on named pipe**
   - Solution: Run BRIDGE with appropriate permissions or change pipe path

2. **TCP connection refused**
   - Solution: Ensure sniffing is started and TCP output is enabled

3. **No data received**
   - Solution: Check that devices are properly connected and communicating

4. **High CPU usage**
   - Solution: Reduce output methods or adjust data format (raw is most efficient)

### Debug Tips

- Use the BRIDGE log to monitor sniffing status
- Check statistics in the Sniffing tab
- Test with simple applications first (like `echo` commands)
- Verify device paths and permissions

## Advanced Configuration

### Custom Paths and Ports
All paths and ports are configurable through the GUI:
- Named pipe path
- TCP server port
- UDP address and port
- Log file location

### Integration with Build Systems
Add BRIDGE sniffing to your development workflow:
- Automated testing with data capture
- CI/CD integration for protocol validation
- Development debugging with real-time monitoring

## Security Considerations

- **Local Access**: Named pipes are local to the system
- **Network Access**: TCP/UDP outputs can be accessed over network
- **Data Sensitivity**: Be aware of what data you're capturing and streaming
- **File Permissions**: Log files inherit system permissions

## Conclusion

The enhanced BRIDGE transforms a simple null modem tool into a professional-grade serial communication analyzer. Whether you're debugging protocols, analyzing device communication, or building complex serial data processing pipelines, BRIDGE provides the flexibility and reliability you need.

For more examples and advanced usage, see the `examples/` directory and the main BRIDGE documentation.
