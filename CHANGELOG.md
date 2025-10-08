# Changelog

All notable changes to the LAST BRIDGE project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Programmable Macro System** - 16 customizable macro buttons for quick command transmission
  - Custom button labels (up to 32 characters each)
  - Command storage (up to 256 characters per command)
  - Persistent settings saved between application launches
  - Professional scrollable layout with 180x30 pixel buttons
- **Network Connectivity** 🆕 - Full TCP/UDP client and server support for Ethernet-based devices
  - **TCP Client**: Connect to remote TCP servers (perfect for NMEA over TCP)
  - **TCP Server**: Accept incoming connections from marine devices or software
  - **UDP Client**: Send data to specific network hosts
  - **UDP Server**: Receive broadcast data from multiple devices
  - **Hostname Resolution**: Support for both IP addresses and hostnames
  - **NMEA 0183 Ready**: Default port 10110 for marine electronics integration
  - **Unified Interface**: Same display, logging, and macro features work with network connections
- **Macro Chaining System** 🆕 - Reference other macros within macro commands
  - Flexible syntax: `{1}` (by number), `{macro1}` (by label), `{Macro 1}` (by custom name)
  - Recursive expansion with infinite loop protection
  - Each referenced macro sends with individual CR-LF line endings
  - **Works with all connection types**: Serial, TCP, and UDP connections
  - Perfect for building complex marine electronics command sequences
- **Macro Panel Toggle** - Show/hide macro panel for flexible screen usage
  - Menu option: "Macros" → "Show/Hide Macro Panel"
  - Dynamic layout adjustment - data areas expand when macros hidden
  - Visibility state persisted between sessions
- **Enhanced File Operations** - Line-by-line file transmission with configurable delays
  - Configurable delays from 100ms to 2000ms between lines
  - Real-time local echo showing transmitted commands
  - Proper device response timing for marine electronics
- **Improved User Interface**
  - Three-panel layout: Settings | Macros | Data Display
  - Dynamic width adjustment for data areas
  - Enhanced menu system with macro management options

### Changed
- **Interface Layout** - Reorganized to accommodate macro panel
  - Left panel: Connection settings, display options, file operations, control signals
  - Middle panel: 16 programmable macro buttons (toggleable)
  - Right panel: Received data, hex data, send data (dynamic width)
- **Settings System** - Extended to support macro configuration persistence
  - Macro labels and commands saved to `~/.config/last/last.conf`
  - Panel visibility state preserved between sessions

### Technical Details
- **Network Module**: New `network.c` and `network.h` with comprehensive TCP/UDP support
- **Unified Connection System**: Replaced `serial_fd` with `connection_fd` for seamless I/O handling
- **Connection Type Management**: Added `ConnectionType` enum and related utility functions
- **Network Thread**: Dedicated `network_read_thread_func()` for non-blocking network I/O
- **Settings Integration**: Extended settings system to save/restore network configurations
- **UI Architecture**: Enhanced connection panel with dynamic frame visibility
- **Macro Architecture**: Added macro chaining support to modular C architecture
- **Constants**: `MAX_MACRO_BUTTONS=16`, `MAX_MACRO_LABEL_LENGTH=32`, `MAX_MACRO_COMMAND_LENGTH=256`
- **UI Components**: GTK3 scrolled window with grid layout for macro buttons
- **Settings**: INI-style configuration with dedicated `[Macros]` and network sections
- **Memory Management**: Efficient button creation, network resource management, and event handling

### Use Cases
- **Marine Electronics Networks**: Connect to GPS units, chart plotters, AIS transponders over Ethernet
- **NMEA 0183 over TCP/UDP**: Receive/send navigation data via network protocols
- **Remote Device Monitoring**: Connect to devices across networks without serial cables
- **Multi-device Networks**: UDP server mode for receiving broadcasts from multiple sources
- **Marine Electronics Testing**: Quick access to fish finder, GPS, and autopilot commands
- **Complex Command Sequences**: Chain multiple commands with proper line endings (works with all connection types)
- **Protocol Development**: Rapid testing of command sequences with macro building blocks
- **Device Configuration**: Store and replay complex configuration commands
- **Modular Command Building**: Create reusable command components that can be combined
- **Network Debugging**: Quick transmission of test patterns and diagnostic commands over TCP/UDP

## [1.0.0] - 2025-01-XX

### Added
- Initial release of LAST BRIDGE suite
- LAST - Linux Advanced Serial Transceiver with GTK3 interface
- BRIDGE - Virtual Null Modem Bridge using socat
- Comprehensive serial port detection overcoming Linux limitations
- Dual text/hex display with dynamic layout
- File operations with repeat sending capability
- Real-time statistics and monitoring
- Professional GUI with modular architecture

### Features
- Advanced port detection (standard, virtual, PTY devices)
- Complete serial parameter control
- Simultaneous text and hex data visualization
- File send/receive operations
- Control signal management (DTR, RTS, Break)
- Data logging and statistics
- Integration between LAST and BRIDGE applications

---

**Copyright © 2025 Fragillidae Software - Chuck Finch**
