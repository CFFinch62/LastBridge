# LAST BRIDGE

[![CI/CD Pipeline](https://github.com/CFFinch62/LastBridge/workflows/CI/CD%20Pipeline/badge.svg)](https://github.com/CFFinch62/LastBridge/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub release](https://img.shields.io/github/release/CFFinch62/LastBridge.svg)](https://github.com/CFFinch62/LastBridge/releases)

A comprehensive serial communication solution consisting of two integrated applications:

- **LAST** - Linux Advanced Serial Transceiver (GTK3 serial terminal)
- **BRIDGE** - Virtual Null Modem Bridge (creates virtual serial devices)

## ✨ Features

### LAST - Linux Advanced Serial Transceiver
- 🔍 **Advanced Port Detection** - Overcomes Linux limitations with comprehensive scanning
- ⚙️ **Complete Serial Control** - All parameters: baud rate, data bits, parity, stop bits
- 📊 **Dual Display Mode** - Simultaneous text and hex data visualization with dynamic layout
- 📁 **File Operations** - Send files with line-by-line transmission and configurable delays
- 🎯 **Programmable Macros** - 16 customizable buttons for quick command transmission
- 📈 **Real-time Statistics** - Data counters, connection time, error tracking
- 🎛️ **Control Signals** - DTR, RTS, Break signal management
- 🎨 **Professional GUI** - Clean, intuitive GTK3 interface with flexible layout
- 🔗 **BRIDGE Integration** - Launch virtual null modem directly from menu

### BRIDGE - Virtual Null Modem Bridge & Serial Sniffer
- 🔗 **Virtual Device Creation** - Creates paired virtual serial devices using socat
- 🕵️ **Universal Serial Sniffing** - Professional-grade serial data capture and analysis
- 📡 **Multiple Output Methods** - Named pipes, TCP sockets, UDP streams, file logging
- 🎯 **Real-time Data Streaming** - Live data feeds for external applications
- 🔍 **Configurable Filtering** - RX only, TX only, or bidirectional capture
- 📊 **Multiple Data Formats** - Raw binary, hex dump, or formatted text output
- ⚙️ **Configurable Paths** - Customize device paths (default: `/tmp/ttyV0` ↔ `/tmp/ttyV1`)
- 🧪 **Communication Testing** - Built-in testing to verify device functionality
- 📊 **Real-time Monitoring** - Process health checking and status display
- 📝 **Comprehensive Logging** - Detailed operation logs with timestamps
- 💾 **Settings Persistence** - Configuration saved between sessions
- 🎨 **Enhanced GUI** - Organized interface with Configuration, Status, Settings, and Sniffing tabs

## 🚀 Quick Start

### One-Line Installation
```bash
curl -sSL https://raw.githubusercontent.com/CFFinch62/LastBridge/main/deploy/install.sh | bash
```

### Manual Installation
```bash
# Clone the repository
git clone https://github.com/CFFinch62/LastBridge.git
cd LastBridge

# Run the installer
./deploy/install.sh
```

### Building from Source
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential libgtk-3-dev socat pkg-config

# Build both applications
make

# Run applications
./LAST/last
./BRIDGE/bridge
```

### Integration Workflow
1. **Start BRIDGE**: Launch and create virtual devices
2. **Start LAST**: Use **Tools → BRIDGE** menu or run separately
3. **Connect**: In LAST, connect to `/tmp/ttyV0` or `/tmp/ttyV1`
4. **Test**: Send data between virtual devices

## 🕵️ Serial Data Sniffing

BRIDGE now includes professional-grade serial data sniffing capabilities, making it a universal serial communication analyzer:

### Sniffing Architecture
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

### Use Cases
- **Marine Electronics**: BBFF video data processing with LAST control
- **Protocol Analysis**: Reverse engineer proprietary serial protocols
- **Device Debugging**: Monitor communication between devices and software
- **Data Logging**: Record serial sessions for offline analysis
- **Multi-app Integration**: One device feeding multiple applications

### Quick Sniffing Setup
1. Launch BRIDGE and go to the "Sniffing" tab
2. Enable sniffing and select output methods (TCP recommended)
3. Start the bridge and begin sniffing
4. Connect your devices to the virtual ports
5. Connect your analysis application to the sniffing output
6. Monitor real-time data flow

For detailed sniffing documentation, see [`BRIDGE/SNIFFING_GUIDE.md`](BRIDGE/SNIFFING_GUIDE.md).

## 📋 System Requirements

### Supported Operating Systems
- Ubuntu 18.04+ / Debian 10+
- Fedora 30+
- Arch Linux
- Other Linux distributions with GTK3

### Dependencies
- **Runtime**: GTK3 libraries, socat utility
- **Build**: GCC, Make, GTK3 development headers, pkg-config

### Hardware
- Any Linux-compatible serial ports
- USB-to-Serial adapters
- Built-in serial ports
- Virtual devices (created by BRIDGE)

## 🔧 Installation

### Automatic Installation (Recommended)
```bash
# Download and run installer
curl -sSL https://raw.githubusercontent.com/CFFinch62/LastBridge/main/deploy/install.sh | bash

# Or clone and install
git clone https://github.com/CFFinch62/LastBridge.git
cd LastBridge
./deploy/install.sh
```

### Package Installation
```bash
# Download release package
wget https://github.com/CFFinch62/LastBridge/releases/latest/download/LastBridge-1.0.0.tar.gz

# Extract and install
tar -xzf LastBridge-1.0.0.tar.gz
cd LastBridge-1.0.0
./deploy/install.sh
```

### Manual Build
```bash
# Install dependencies
sudo apt-get install build-essential libgtk-3-dev socat pkg-config

# Build
make clean && make

# Install (optional)
sudo make install
```

## 🎯 Usage Examples

### Basic Serial Communication
1. Start LAST: `last` or find in applications menu
2. Refresh ports to detect available devices
3. Select port, configure parameters
4. Click Connect
5. Send/receive data in terminal

### Virtual Null Modem Testing
1. Start BRIDGE: `bridge`
2. Configure device paths (default: `/tmp/ttyV0`, `/tmp/ttyV1`)
3. Click "Start Virtual Null Modem"
4. Start LAST (or use Tools → BRIDGE menu)
5. Connect to virtual devices for testing

### File Operations
- **Send File**: Use File menu in LAST to send files over serial
- **Save Data**: Capture received data to files
- **Logging**: Enable logging for session recording

## 🏗️ Architecture

Both applications follow a modular C architecture:

```
src/
├── common.h          # Shared includes and structures
├── main.c            # Application initialization
├── ui.c/.h           # GTK3 user interface
├── callbacks.c/.h    # Event handlers
├── settings.c/.h     # Configuration management
├── utils.c/.h        # Utility functions
└── [app-specific modules]
```

### LAST Modules
- `serial.c/.h` - Serial port operations and communication
- `file_ops.c/.h` - File send/receive operations

### BRIDGE Modules
- `nullmodem.c/.h` - Virtual device management using socat

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Quick Contribution Guide
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes and test thoroughly
4. Commit: `git commit -m 'Add amazing feature'`
5. Push: `git push origin feature/amazing-feature`
6. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- GTK3 development team for the excellent GUI framework
- socat developers for the versatile networking tool
- Linux serial subsystem maintainers
- Open source community for inspiration and feedback

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/CFFinch62/LastBridge/issues)
- **Discussions**: [GitHub Discussions](https://github.com/CFFinch62/LastBridge/discussions)
- **Email**: cffinch62@example.com

## 🔗 Links

- **Documentation**: [Wiki](https://github.com/CFFinch62/LastBridge/wiki)
- **Releases**: [GitHub Releases](https://github.com/CFFinch62/LastBridge/releases)
- **Changelog**: [CHANGELOG.md](CHANGELOG.md)

---

**Copyright © 2025 Fragillidae Software - Chuck Finch**

Made with ❤️ for the Linux community
