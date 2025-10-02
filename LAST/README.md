# LAST - Linux Advanced Serial Trasncevier

A full-featured serial terminal application built from the working low-level serial terminal code. This application provides all the features of professional serial terminals like cutecom and moserial, while overcoming their Linux port enumeration limitations.

## 🎯 **Key Features**

### **Superior Port Detection**
- ✅ **Finds ALL serial devices** - not limited by standard enumeration
- ✅ **Virtual port support** - works with PyVComm and other virtual devices
- ✅ **Comprehensive scanning** - checks `/dev/`, `/tmp/`, `/dev/pts/`
- ✅ **Real device testing** - uses actual system calls to verify accessibility

### **Complete Serial Parameter Control**
- ✅ **Baud rates**: 300 to 921600 bps
- ✅ **Data bits**: 5, 6, 7, 8
- ✅ **Parity**: None, Even, Odd
- ✅ **Stop bits**: 1, 2
- ✅ **Flow control**: None, Hardware (RTS/CTS), Software (XON/XOFF)

### **Advanced Display Features**
- ✅ **Dual display mode** - simultaneous text and hex data display 🆕
- ✅ **Hex display mode** - view data in hexadecimal format with configurable bytes per line
- ✅ **Dynamic layout** - hex display toggles on/off with automatic window resizing
- ✅ **Timestamps** - add timestamps to all received data (both displays)
- ✅ **Auto-scroll** - automatically scroll to show latest data (both displays)
- ✅ **Local echo** - echo sent data in receive area
- ✅ **Line ending control** - None, CR, LF, CR+LF

### **File Operations**
- ✅ **Send files** - transmit entire files through serial port
- ✅ **Repeat file sending** - automatically repeat file transmission at configurable intervals
- ✅ **Flexible timing** - choose from 0.1, 0.2, 0.5, 1.0, or 2.0 second intervals
- ✅ **Save received data** - save all received data to text files
- ✅ **Data logging** - log all communication to files with timestamps

### **Control Signals**
- ✅ **DTR control** - Data Terminal Ready signal
- ✅ **RTS control** - Request To Send signal
- ✅ **Break signal** - send break signal

### **Statistics & Monitoring**
- ✅ **Bytes sent/received** - real-time statistics
- ✅ **Connection time** - track connection duration
- ✅ **Status updates** - clear connection status display

## 🚀 **Quick Start**

### **Building**
The application uses a modular architecture with multiple source files:

```bash
cd LAST
make
```

The build system automatically:
- Compiles all source files in the `src/` directory
- Handles dependencies between modules
- Links with required GTK3 and pthread libraries
- Creates the `last` executable

### **Running**
```bash
./last
```

### **Using with PyVComm**
1. Start PyVComm: `python3 ../pyvcomm_gui.py`
2. Click "Start Virtual Null Modem" in PyVComm
3. Start the terminal: `./last`
4. Click "Refresh" - you'll see `/tmp/ttyV0` and `/tmp/ttyV1`
5. Connect to either port and test communication

## 🎯 **Interface Layout**

### **Left Panel - Settings**
- **Connection Settings**: Port, baud rate, data bits, parity, stop bits, flow control
- **Display Options**: Hex display, timestamps, auto-scroll, local echo, line endings
- **File Operations**: Send file, save data, logging controls
- **Control Signals**: DTR, RTS, break signal

### **Right Panel - Data**
- **Received Data Area**: Scrollable text area for received data in text format
- **Hex Data Area**: Scrollable hex display (shown by default, toggleable)
- **Send Area**: Text entry and send button for outgoing data

### **Status Bar**
- **Connection status**: Current port and settings
- **Statistics**: Bytes sent/received, connection time

## 🔧 **Advanced Features**

### **Dual Text/Hex Display** 🆕
Experience simultaneous text and hex data visualization:

1. **Default Layout** - Both text and hex displays are shown by default
2. **Synchronized Data** - All received data appears in both formats simultaneously
3. **Toggle Control** - Use View → Display Options → "Hex Display" to show/hide hex area
4. **Dynamic Resizing** - Text area automatically adjusts height when hex display is toggled
5. **Consistent Styling** - Both displays share the same fonts, colors, and appearance settings
6. **Unified Operations** - Clear and Save functions work on both displays

**Layout Behavior:**
- **Hex Display ON**: Text area (120px) + Hex area (120px) = Split view
- **Hex Display OFF**: Text area (240px) = Full height view

**Use Cases:**
- **Protocol Analysis** - See both human-readable text and raw hex data
- **Binary Data Inspection** - Examine non-printable characters in hex format
- **Debugging** - Compare expected vs actual data in both formats
- **Data Validation** - Verify data integrity across different representations

### **Repeat File Sending** 🆕
Perfect for testing and simulation scenarios:

1. **Select a file** - choose any text file to transmit
2. **Enable repeat mode** - check "Repeat sending" checkbox
3. **Choose interval** - select from 0.1, 0.2, 0.5, 1.0, or 2.0 seconds
4. **Start transmission** - file sends immediately, then repeats at chosen interval
5. **Stop anytime** - click "Stop Repeat" to halt transmission

**Use Cases:**
- **NMEA sentence simulation** - repeatedly send GPS data for testing
- **Protocol testing** - send command sequences at regular intervals
- **Data streaming** - simulate continuous sensor data
- **Stress testing** - flood test with repeated data

### **Port Detection Algorithm**
1. **Standard enumeration** - uses system port detection
2. **Direct scanning** - scans `/dev/tty*`, `/dev/ttyS*`, `/dev/ttyUSB*`, etc.
3. **Virtual device detection** - checks `/tmp/ttyV*` for PyVComm
4. **PTY scanning** - enumerates `/dev/pts/` directory
5. **Accessibility testing** - actually tries to open devices

### **Why This Works Better**
- **Direct system calls** - bypasses Qt/GTK filtering
- **Comprehensive coverage** - checks all possible locations
- **Real testing** - verifies device accessibility
- **No artificial restrictions** - includes virtual and non-standard devices

## 📋 **Comparison with Other Terminals**

| Feature | cutecom | moserial | LAST Terminal |
|---------|---------|----------|--------------------------|
| Standard ports | ✅ | ✅ | ✅ |
| Virtual ports | ❌ | ❌ | ✅ |
| PyVComm support | ❌ | ❌ | ✅ |
| Custom paths | ❌ | ❌ | ✅ |
| Full parameter control | ✅ | ✅ | ✅ |
| Hex display | ✅ | ✅ | ✅ |
| **Dual text/hex display** | ❌ | ❌ | ✅ 🆕 |
| **Dynamic layout** | ❌ | ❌ | ✅ 🆕 |
| Timestamps | ❌ | ✅ | ✅ |
| File operations | ✅ | ✅ | ✅ |
| **Repeat file sending** | ❌ | ❌ | ✅ 🆕 |
| **Configurable intervals** | ❌ | ❌ | ✅ 🆕 |
| Data logging | ❌ | ✅ | ✅ |
| Control signals | ✅ | ✅ | ✅ |
| Statistics | ❌ | ❌ | ✅ |

## 🛠 **Technical Details**

### **Built From Working Code**
This application is built by enhancing the proven low-level serial terminal that already works. The codebase has been refactored into a modular architecture for better maintainability:

1. ✅ **Started with working port detection**
2. ✅ **Added professional GUI features**
3. ✅ **Enhanced with complete parameter control**
4. ✅ **Integrated file operations and logging**
5. ✅ **Refactored into modular design** - easier development and debugging

### **Dependencies**
- **GTK3** - GUI framework
- **pthread** - threading support
- **Standard C libraries** - system calls and utilities

### **Architecture**
- **Modular design** - separate source files for each functional area
- **Clean separation** - well-defined module responsibilities
- **Event-driven GUI** - responsive interface
- **Threaded I/O** - non-blocking serial communication

#### **Module Structure**
```
src/
├── common.h            # Shared includes, constants, and main struct
├── main.c              # Application initialization and coordination
├── serial.c/.h         # Serial port operations and communication
├── ui.c/.h             # GTK user interface creation
├── file_ops.c/.h       # File operations (send, save, log)
├── utils.c/.h          # Utility functions (formatting, timestamps, statistics)
└── callbacks.c/.h      # GTK event handlers
```

#### **Module Responsibilities**
- **main.c**: Application entry point, initialization, and module coordination
- **serial.c**: Port detection, connection management, configuration, and I/O operations
- **ui.c**: GTK interface creation, layout, and widget management
- **file_ops.c**: File sending, saving received data, and logging functionality
- **utils.c**: Data formatting, timestamp generation, and statistics management
- **callbacks.c**: GTK event handling and UI interactions

#### **Benefits of Modular Design**
- **Easier development** - work on individual components independently
- **Simplified debugging** - isolate issues to specific functional areas
- **Better maintainability** - changes to one area don't affect others
- **Improved readability** - code organized by functionality
- **Enhanced scalability** - new features can be added as separate modules

## 🎉 **Success Story**

This application solves the fundamental problem with Linux serial terminals:

**Problem**: cutecom, moserial, and other GUI terminals use Qt/GTK port enumeration that misses virtual devices and has artificial filtering.

**Solution**: Direct device scanning with real accessibility testing finds ALL serial devices, including virtual ports that other programs miss.

**Result**: A professional serial terminal that actually works with PyVComm and other virtual serial solutions!

## 📄 **License**

Part of the PyVComm project suite.

---

**You now have a complete, professional serial terminal that overcomes Linux enumeration limitations!** 🚀
