#!/bin/bash

# LAST BRIDGE Installation Script
# Copyright (c) 2025 Fragillidae Software - Chuck Finch
# Licensed under MIT License

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
INSTALL_PREFIX="/usr/local"
DESKTOP_DIR="/usr/share/applications"
ICON_DIR="/usr/share/pixmaps"

echo -e "${BLUE}LAST BRIDGE Installer${NC}"
echo -e "${BLUE}=====================${NC}"
echo ""
echo "This script will install LAST and BRIDGE applications."
echo "Installation prefix: $INSTALL_PREFIX"
echo ""

# Check if running as root for system-wide installation
if [[ $EUID -eq 0 ]]; then
    echo -e "${YELLOW}Running as root - installing system-wide${NC}"
    INSTALL_USER_ONLY=false
else
    echo -e "${YELLOW}Running as user - you may need sudo for system installation${NC}"
    echo "Choose installation type:"
    echo "1) System-wide installation (requires sudo)"
    echo "2) User-only installation (~/.local)"
    read -p "Enter choice [1-2]: " choice
    
    case $choice in
        1)
            INSTALL_USER_ONLY=false
            echo -e "${YELLOW}Will request sudo privileges when needed${NC}"
            ;;
        2)
            INSTALL_USER_ONLY=true
            INSTALL_PREFIX="$HOME/.local"
            DESKTOP_DIR="$HOME/.local/share/applications"
            ICON_DIR="$HOME/.local/share/pixmaps"
            echo -e "${GREEN}Installing to user directory: $INSTALL_PREFIX${NC}"
            ;;
        *)
            echo -e "${RED}Invalid choice. Exiting.${NC}"
            exit 1
            ;;
    esac
fi

# Function to run command with sudo if needed
run_with_sudo() {
    if [[ $INSTALL_USER_ONLY == true ]] || [[ $EUID -eq 0 ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

echo ""
echo -e "${BLUE}Step 1: Checking dependencies${NC}"

# Check for required tools
MISSING_DEPS=()

if ! command_exists gcc; then
    MISSING_DEPS+=("gcc")
fi

if ! command_exists make; then
    MISSING_DEPS+=("make")
fi

if ! command_exists pkg-config; then
    MISSING_DEPS+=("pkg-config")
fi

if ! command_exists socat; then
    MISSING_DEPS+=("socat")
fi

# Check for GTK3 development libraries
if ! pkg-config --exists gtk+-3.0; then
    MISSING_DEPS+=("libgtk-3-dev")
fi

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${RED}Missing dependencies: ${MISSING_DEPS[*]}${NC}"
    echo ""
    echo "Please install missing dependencies:"
    echo ""
    echo -e "${YELLOW}Ubuntu/Debian:${NC}"
    echo "sudo apt-get update"
    echo "sudo apt-get install build-essential libgtk-3-dev socat pkg-config"
    echo ""
    echo -e "${YELLOW}Fedora:${NC}"
    echo "sudo dnf install gcc make gtk3-devel socat pkgconf-pkg-config"
    echo ""
    echo -e "${YELLOW}Arch Linux:${NC}"
    echo "sudo pacman -S base-devel gtk3 socat pkgconf"
    echo ""
    exit 1
fi

echo -e "${GREEN}✓ All dependencies found${NC}"

echo ""
echo -e "${BLUE}Step 2: Building applications${NC}"

# Navigate to project root (assuming script is in deploy/ subdirectory)
cd "$(dirname "$0")/.."

# Clean and build
echo "Cleaning previous builds..."
make clean >/dev/null 2>&1

echo "Building LAST and BRIDGE..."
if ! make; then
    echo -e "${RED}Build failed. Please check the error messages above.${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build successful${NC}"

echo ""
echo -e "${BLUE}Step 3: Installing applications${NC}"

# Create directories
echo "Creating installation directories..."
run_with_sudo mkdir -p "$INSTALL_PREFIX/bin"
run_with_sudo mkdir -p "$DESKTOP_DIR"
run_with_sudo mkdir -p "$ICON_DIR"

# Install binaries
echo "Installing LAST..."
run_with_sudo cp LAST/last "$INSTALL_PREFIX/bin/"
run_with_sudo chmod +x "$INSTALL_PREFIX/bin/last"

echo "Installing BRIDGE..."
run_with_sudo cp BRIDGE/bridge "$INSTALL_PREFIX/bin/"
run_with_sudo chmod +x "$INSTALL_PREFIX/bin/bridge"

echo -e "${GREEN}✓ Binaries installed to $INSTALL_PREFIX/bin/${NC}"

echo ""
echo -e "${BLUE}Step 4: Creating desktop entries${NC}"

# Create desktop entries
cat > /tmp/last.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=LAST
Comment=Linux Advanced Serial Transceiver
Exec=$INSTALL_PREFIX/bin/last
Icon=last
Terminal=false
Categories=Development;Electronics;
Keywords=serial;terminal;communication;
EOF

cat > /tmp/bridge.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=BRIDGE
Comment=Virtual Null Modem Bridge
Exec=$INSTALL_PREFIX/bin/bridge
Icon=bridge
Terminal=false
Categories=Development;Electronics;
Keywords=serial;nullmodem;virtual;
EOF

run_with_sudo cp /tmp/last.desktop "$DESKTOP_DIR/"
run_with_sudo cp /tmp/bridge.desktop "$DESKTOP_DIR/"

# Clean up temporary files
rm /tmp/last.desktop /tmp/bridge.desktop

echo -e "${GREEN}✓ Desktop entries created${NC}"

echo ""
echo -e "${BLUE}Step 5: Installation complete${NC}"
echo ""
echo -e "${GREEN}LAST BRIDGE has been successfully installed!${NC}"
echo ""
echo "Installed applications:"
echo "  • LAST (Linux Advanced Serial Transceiver): $INSTALL_PREFIX/bin/last"
echo "  • BRIDGE (Virtual Null Modem Bridge): $INSTALL_PREFIX/bin/bridge"
echo ""
echo "You can now:"
echo "  • Run applications from command line: 'last' or 'bridge'"
echo "  • Find them in your application menu under Development"
echo "  • Use Tools → BRIDGE menu in LAST to launch BRIDGE"
echo ""
echo "For usage instructions, run: 'last --help' or 'bridge --help'"
echo "Or visit: https://github.com/fragillidae-software/last-bridge"
echo ""
echo -e "${YELLOW}Note: You may need to log out and back in for desktop entries to appear.${NC}"
