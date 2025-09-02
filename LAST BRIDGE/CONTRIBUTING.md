# Contributing to LAST BRIDGE

Thank you for your interest in contributing to LAST BRIDGE! This document provides guidelines for contributing to both LAST (Linux Advanced Serial Transceiver) and BRIDGE (Virtual Null Modem Bridge).

## 🚀 Getting Started

### Prerequisites

- Linux development environment
- GCC compiler and Make
- GTK3 development libraries
- socat utility
- Git

### Setting Up Development Environment

1. **Fork and Clone**
   ```bash
   git clone https://github.com/your-username/last-bridge.git
   cd last-bridge
   ```

2. **Install Dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install build-essential libgtk-3-dev socat pkg-config git
   
   # Fedora
   sudo dnf install gcc make gtk3-devel socat pkgconf-pkg-config git
   
   # Arch Linux
   sudo pacman -S base-devel gtk3 socat pkgconf git
   ```

3. **Build and Test**
   ```bash
   make clean && make
   ./LAST/last
   ./BRIDGE/bridge
   ```

## 📋 How to Contribute

### Reporting Issues

1. **Search Existing Issues** - Check if the issue already exists
2. **Use Issue Templates** - Follow the provided templates
3. **Provide Details** - Include:
   - Operating system and version
   - GTK3 version
   - Steps to reproduce
   - Expected vs actual behavior
   - Error messages or logs

### Suggesting Features

1. **Check Roadmap** - Review existing feature requests
2. **Open Discussion** - Create an issue with the "enhancement" label
3. **Provide Context** - Explain the use case and benefits

### Code Contributions

1. **Create Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Follow Coding Standards** (see below)

3. **Test Your Changes**
   ```bash
   make clean && make
   # Test both applications thoroughly
   ```

4. **Commit with Clear Messages**
   ```bash
   git commit -m "Add feature: brief description
   
   Detailed explanation of what this commit does and why."
   ```

5. **Push and Create Pull Request**
   ```bash
   git push origin feature/your-feature-name
   ```

## 🎯 Coding Standards

### C Code Style

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 100 characters
- **Naming Conventions**:
  - Functions: `snake_case`
  - Variables: `snake_case`
  - Constants: `UPPER_CASE`
  - Structs: `PascalCase`

- **Comments**: Use clear, descriptive comments
  ```c
  // Good: Calculate baud rate from user input
  int baud_rate = calculate_baud_rate(user_input);
  
  // Bad: calc br
  int br = calc_br(ui);
  ```

### Architecture Guidelines

- **Modular Design**: Follow the existing modular structure
- **Separation of Concerns**: Keep UI, logic, and utilities separate
- **Error Handling**: Always check return values and handle errors gracefully
- **Memory Management**: Use proper allocation/deallocation patterns
- **GTK3 Patterns**: Follow GTK3 best practices for UI code

### File Organization

```
src/
├── common.h          # Shared definitions
├── main.c            # Application entry point
├── ui.c/.h           # User interface
├── callbacks.c/.h    # Event handlers
├── settings.c/.h     # Configuration management
├── utils.c/.h        # Utility functions
└── [module].c/.h     # Specific functionality
```

## 🧪 Testing

### Manual Testing

1. **Build Testing**
   ```bash
   make clean && make
   ```

2. **Functionality Testing**
   - Test all UI components
   - Verify serial port operations
   - Test virtual null modem creation
   - Check settings persistence
   - Test integration between LAST and BRIDGE

3. **Error Handling**
   - Test with invalid inputs
   - Test with missing dependencies
   - Test permission scenarios

### Integration Testing

1. **LAST ↔ BRIDGE Integration**
   - Start BRIDGE, create virtual devices
   - Launch LAST from BRIDGE menu
   - Connect to virtual devices
   - Test data transmission

2. **Cross-Platform Testing**
   - Test on different Linux distributions
   - Verify GTK3 compatibility
   - Check dependency handling

## 📝 Documentation

### Code Documentation

- **Function Headers**: Document all public functions
  ```c
  /**
   * Creates a new serial connection
   * @param port_name: Name of the serial port (e.g., "/dev/ttyUSB0")
   * @param baud_rate: Baud rate for communication
   * @return: File descriptor on success, -1 on error
   */
  int create_serial_connection(const char *port_name, int baud_rate);
  ```

- **Complex Logic**: Explain non-obvious code sections
- **TODOs**: Mark areas for future improvement

### User Documentation

- Update README.md for new features
- Add usage examples
- Update help text and tooltips
- Document configuration options

## 🔄 Pull Request Process

1. **Pre-submission Checklist**
   - [ ] Code builds without warnings
   - [ ] All functionality tested
   - [ ] Documentation updated
   - [ ] Commit messages are clear
   - [ ] No debugging code left in

2. **Pull Request Description**
   - Describe what the PR does
   - Reference related issues
   - Include testing notes
   - Add screenshots for UI changes

3. **Review Process**
   - Maintainer will review within 48 hours
   - Address feedback promptly
   - Keep PR focused and atomic

## 🏗️ Development Areas

### High Priority
- Serial port auto-detection improvements
- Enhanced error handling
- Performance optimizations
- Additional virtual device types

### Medium Priority
- Configuration import/export
- Logging enhancements
- UI theme support
- Plugin architecture

### Low Priority
- Windows port (via WSL)
- Additional protocols
- Scripting support
- Remote device support

## 🤝 Community Guidelines

- **Be Respectful**: Treat all contributors with respect
- **Be Constructive**: Provide helpful feedback
- **Be Patient**: Allow time for reviews and responses
- **Be Collaborative**: Work together to improve the project

## 📞 Getting Help

- **GitHub Issues**: For bugs and feature requests
- **GitHub Discussions**: For questions and general discussion
- **Email**: fragillidae.software@example.com (for sensitive issues)

## 📄 License

By contributing to this project, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to LAST BRIDGE! 🚀
