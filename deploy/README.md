# LAST BRIDGE - Deployment

This directory contains installation and deployment materials for LAST BRIDGE.

## Quick Installation

### Automatic Installation (Recommended)

```bash
# Clone the repository
git clone https://github.com/fragillidae-software/last-bridge.git
cd last-bridge

# Run the installer
./deploy/install.sh
```

The installer will:
- Check for dependencies
- Build both applications
- Install binaries to `/usr/local/bin` (or `~/.local/bin` for user install)
- Create desktop entries
- Set up proper permissions

### Manual Installation

If you prefer to install manually:

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential libgtk-3-dev socat pkg-config

# Build applications
make clean && make

# Install system-wide (requires sudo)
sudo make install

# Or install to user directory
make install PREFIX=$HOME/.local
```

## Dependencies

### Required Packages

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential libgtk-3-dev socat pkg-config
```

**Fedora:**
```bash
sudo dnf install gcc make gtk3-devel socat pkgconf-pkg-config
```

**Arch Linux:**
```bash
sudo pacman -S base-devel gtk3 socat pkgconf
```

### Runtime Dependencies
- GTK3 libraries
- socat utility
- Standard C library

### Build Dependencies
- GCC compiler
- Make build system
- GTK3 development headers
- pkg-config

## Installation Options

### System-wide Installation
- Installs to `/usr/local/bin/`
- Available to all users
- Requires sudo privileges
- Desktop entries in `/usr/share/applications/`

### User Installation
- Installs to `~/.local/bin/`
- Available only to current user
- No sudo required
- Desktop entries in `~/.local/share/applications/`

## Uninstallation

To remove LAST BRIDGE:

```bash
./deploy/uninstall.sh
```

The uninstaller will:
- Detect installation type (system/user)
- Remove binaries and desktop entries
- Optionally remove configuration files
- Clean up all installed components

## Package Creation

### Creating a Debian Package

```bash
# Install packaging tools
sudo apt-get install devscripts debhelper

# Create package
./deploy/create-deb.sh
```

### Creating an RPM Package

```bash
# Install packaging tools (Fedora)
sudo dnf install rpm-build rpmdevtools

# Create package
./deploy/create-rpm.sh
```

### Creating a Tarball Distribution

```bash
# Create source distribution
./deploy/create-tarball.sh
```

## Deployment Structure

```
deploy/
├── README.md           # This file
├── install.sh          # Automatic installer script
├── uninstall.sh        # Uninstaller script
├── create-deb.sh       # Debian package creator
├── create-rpm.sh       # RPM package creator
├── create-tarball.sh   # Source tarball creator
├── debian/             # Debian packaging files
├── rpm/                # RPM packaging files
└── icons/              # Application icons
```

## Troubleshooting

### Common Issues

**Build Fails:**
- Ensure all dependencies are installed
- Check that GTK3 development headers are available
- Verify GCC and make are installed

**Permission Denied:**
- Use sudo for system-wide installation
- Or choose user installation option

**Desktop Entries Don't Appear:**
- Log out and back in
- Or run: `update-desktop-database`

**BRIDGE Launch Fails from LAST:**
- Ensure both applications are in PATH
- Check that BRIDGE is executable
- Verify relative path is correct

### Getting Help

1. Check the main README.md for usage instructions
2. Run applications with `--help` flag
3. Check GitHub issues: https://github.com/fragillidae-software/last-bridge/issues
4. Contact: Fragillidae Software

## License

This deployment package is part of LAST BRIDGE, licensed under the MIT License. See the LICENSE file in the project root for details.

Copyright (c) 2025 Fragillidae Software - Chuck Finch
