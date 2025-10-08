# LAST - Changelog

All notable changes to the LAST (Linux Advanced Serial Transceiver) project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-01-08

### Added
- **Lua Scripting Window Interface**: Added dedicated scripting window accessible via Tools → "Lua Scripting..."
  - Full-featured script editor with monospace font and syntax highlighting support
  - Toolbar with Enable/Disable, Load, Save, Test, and Clear script functions
  - File operations with .lua file filter for loading and saving scripts
  - Script validation and testing with user feedback
  - Information panel showing available functions and event handlers
  - Proper window management preventing multiple instances
  - Integration with existing Lua scripting engine

### Changed
- Updated application version to 1.1.0
- Enhanced About dialog to mention Lua scripting engine and dedicated window
- Improved scripting engine initialization and cleanup

### Technical Details
- Added `create_scripting_window()` function in ui.c
- Implemented complete set of scripting callback functions in callbacks.c
- Added proper scripting engine initialization in main.c
- Enhanced error handling and user feedback for script operations
- Integrated scripting cleanup with application lifecycle management

## [1.0.0] - 2025-01-07

### Added
- Initial release of LAST - Linux Advanced Serial Transceiver
- Complete serial terminal functionality with superior port detection
- Network connectivity (TCP/UDP client/server modes)
- Embedded Lua 5.4.7 scripting engine with marine electronics API
- Dual text/hex display with dynamic layout
- Programmable macro system with 16 customizable buttons
- File operations including repeat sending and line-by-line transmission
- Professional GTK3 interface with comprehensive settings
- Integration with BRIDGE virtual null modem
- Comprehensive documentation and scripting guide

### Features
- Superior port detection finding ALL serial devices including virtual ports
- Complete serial parameter control (baud rate, data bits, parity, stop bits, flow control)
- Real-time statistics and connection monitoring
- Data logging and file operations
- Control signals (DTR, RTS, Break)
- Appearance customization (themes, fonts, colors)
- Settings persistence between sessions

### Documentation
- Comprehensive README.md with feature overview and usage instructions
- Detailed SCRIPTING_GUIDE.md with API documentation and examples
- Example Lua scripts demonstrating marine electronics applications
