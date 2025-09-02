#!/bin/bash

# LAST BRIDGE - Source Distribution Creator
# Copyright (c) 2025 Fragillidae Software - Chuck Finch
# Licensed under MIT License

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="last-bridge"
VERSION="1.0.0"
DIST_DIR="dist"

echo -e "${BLUE}LAST BRIDGE - Source Distribution Creator${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""

# Navigate to project root
cd "$(dirname "$0")/.."

# Clean any previous builds
echo "Cleaning previous builds..."
make clean >/dev/null 2>&1 || true

# Create distribution directory
echo "Creating distribution directory..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Create source package
PACKAGE_NAME="${PROJECT_NAME}-${VERSION}"
PACKAGE_DIR="$DIST_DIR/$PACKAGE_NAME"

echo "Creating source package: $PACKAGE_NAME"
mkdir -p "$PACKAGE_DIR"

# Copy source files
echo "Copying source files..."
cp -r LAST/ "$PACKAGE_DIR/"
cp -r BRIDGE/ "$PACKAGE_DIR/"
cp -r deploy/ "$PACKAGE_DIR/"
cp Makefile "$PACKAGE_DIR/"
cp README.md "$PACKAGE_DIR/"
cp LICENSE "$PACKAGE_DIR/"
cp .gitignore "$PACKAGE_DIR/"

# Clean any build artifacts from copied directories
find "$PACKAGE_DIR" -name "*.o" -delete
find "$PACKAGE_DIR" -name "last" -delete
find "$PACKAGE_DIR" -name "bridge" -delete

# Create tarball
echo "Creating tarball..."
cd "$DIST_DIR"
tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"

# Create zip file for Windows users
echo "Creating zip file..."
zip -r "${PACKAGE_NAME}.zip" "$PACKAGE_NAME" >/dev/null

# Calculate checksums
echo "Calculating checksums..."
sha256sum "${PACKAGE_NAME}.tar.gz" > "${PACKAGE_NAME}.tar.gz.sha256"
sha256sum "${PACKAGE_NAME}.zip" > "${PACKAGE_NAME}.zip.sha256"

# Create release info
cat > "${PACKAGE_NAME}-RELEASE-INFO.txt" << EOF
LAST BRIDGE v${VERSION}
=======================

Release Date: $(date '+%Y-%m-%d')
Package Contents:
  • LAST - Linux Advanced Serial Transceiver
  • BRIDGE - Virtual Null Modem Bridge
  • Complete source code
  • Build system (Makefile)
  • Installation scripts
  • Documentation

Files in this release:
  • ${PACKAGE_NAME}.tar.gz     - Source code (Linux/Unix)
  • ${PACKAGE_NAME}.zip        - Source code (Windows)
  • ${PACKAGE_NAME}.tar.gz.sha256 - Checksum for tarball
  • ${PACKAGE_NAME}.zip.sha256    - Checksum for zip file

Installation:
  1. Extract the archive
  2. Run: ./deploy/install.sh
  
For manual installation:
  1. Install dependencies (see README.md)
  2. Run: make && sudo make install

System Requirements:
  • Linux with GTK3
  • GCC compiler
  • Make build system
  • socat utility

License: MIT License
Author: Fragillidae Software - Chuck Finch
Website: https://github.com/fragillidae-software/last-bridge

EOF

cd ..

# Display results
echo ""
echo -e "${GREEN}Source distribution created successfully!${NC}"
echo ""
echo "Created files in $DIST_DIR/:"
ls -la "$DIST_DIR"
echo ""
echo "Package size:"
du -h "$DIST_DIR/${PACKAGE_NAME}.tar.gz"
du -h "$DIST_DIR/${PACKAGE_NAME}.zip"
echo ""
echo -e "${YELLOW}Ready for distribution!${NC}"
