#!/bin/bash

# LAST BRIDGE Uninstaller
# Copyright (c) 2025 Fragillidae Software - Chuck Finch
# Licensed under MIT License

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}LAST BRIDGE Uninstaller${NC}"
echo -e "${BLUE}=======================${NC}"
echo ""

# Function to run command with sudo if needed
run_with_sudo() {
    if [[ $EUID -eq 0 ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

# Check for system-wide installation
SYSTEM_INSTALL=false
USER_INSTALL=false

if [[ -f "/usr/local/bin/last" ]] || [[ -f "/usr/local/bin/bridge" ]]; then
    SYSTEM_INSTALL=true
fi

if [[ -f "$HOME/.local/bin/last" ]] || [[ -f "$HOME/.local/bin/bridge" ]]; then
    USER_INSTALL=true
fi

if [[ $SYSTEM_INSTALL == false ]] && [[ $USER_INSTALL == false ]]; then
    echo -e "${YELLOW}No installation found.${NC}"
    echo "LAST BRIDGE does not appear to be installed."
    exit 0
fi

echo "Found installations:"
if [[ $SYSTEM_INSTALL == true ]]; then
    echo "  • System-wide installation (/usr/local)"
fi
if [[ $USER_INSTALL == true ]]; then
    echo "  • User installation (~/.local)"
fi
echo ""

# Confirm uninstallation
read -p "Are you sure you want to uninstall LAST BRIDGE? [y/N]: " confirm
if [[ ! $confirm =~ ^[Yy]$ ]]; then
    echo "Uninstallation cancelled."
    exit 0
fi

echo ""
echo -e "${BLUE}Removing LAST BRIDGE...${NC}"

# Remove system-wide installation
if [[ $SYSTEM_INSTALL == true ]]; then
    echo "Removing system-wide installation..."
    
    # Remove binaries
    run_with_sudo rm -f /usr/local/bin/last
    run_with_sudo rm -f /usr/local/bin/bridge
    
    # Remove desktop entries
    run_with_sudo rm -f /usr/share/applications/last.desktop
    run_with_sudo rm -f /usr/share/applications/bridge.desktop
    
    # Remove icons (if they exist)
    run_with_sudo rm -f /usr/share/pixmaps/last.png
    run_with_sudo rm -f /usr/share/pixmaps/bridge.png
    run_with_sudo rm -f /usr/share/pixmaps/last.svg
    run_with_sudo rm -f /usr/share/pixmaps/bridge.svg
    
    echo -e "${GREEN}✓ System-wide installation removed${NC}"
fi

# Remove user installation
if [[ $USER_INSTALL == true ]]; then
    echo "Removing user installation..."
    
    # Remove binaries
    rm -f "$HOME/.local/bin/last"
    rm -f "$HOME/.local/bin/bridge"
    
    # Remove desktop entries
    rm -f "$HOME/.local/share/applications/last.desktop"
    rm -f "$HOME/.local/share/applications/bridge.desktop"
    
    # Remove icons (if they exist)
    rm -f "$HOME/.local/share/pixmaps/last.png"
    rm -f "$HOME/.local/share/pixmaps/bridge.png"
    rm -f "$HOME/.local/share/pixmaps/last.svg"
    rm -f "$HOME/.local/share/pixmaps/bridge.svg"
    
    echo -e "${GREEN}✓ User installation removed${NC}"
fi

# Ask about configuration files
echo ""
read -p "Remove user configuration files? [y/N]: " remove_config
if [[ $remove_config =~ ^[Yy]$ ]]; then
    rm -f "$HOME/.last_config"
    rm -f "$HOME/.bridge_config"
    echo -e "${GREEN}✓ Configuration files removed${NC}"
else
    echo -e "${YELLOW}Configuration files preserved${NC}"
fi

echo ""
echo -e "${GREEN}LAST BRIDGE has been successfully uninstalled!${NC}"
echo ""
echo -e "${YELLOW}Note: You may need to log out and back in for desktop entries to disappear.${NC}"
