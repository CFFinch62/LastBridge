# Makefile for BRIDGE - Virtual Null Modem Bridge

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -g
TARGET = bridge

# GTK3 configuration
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

# Source files
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/nullmodem.c $(SRCDIR)/ui.c $(SRCDIR)/utils.c $(SRCDIR)/callbacks.c $(SRCDIR)/settings.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SRCDIR)/common.h $(SRCDIR)/nullmodem.h $(SRCDIR)/ui.h $(SRCDIR)/utils.h $(SRCDIR)/callbacks.h $(SRCDIR)/settings.h

.PHONY: all clean install uninstall run check-deps help

all: check-deps $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(GTK_LIBS) -lpthread

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -I$(SRCDIR) -c $< -o $@

check-deps:
	@echo "Checking dependencies..."
	@pkg-config --exists gtk+-3.0 || (echo "ERROR: GTK3 development libraries not found. Install with:" && echo "  Ubuntu/Debian: sudo apt-get install libgtk-3-dev" && echo "  Fedora: sudo dnf install gtk3-devel" && echo "  Arch: sudo pacman -S gtk3" && exit 1)
	@which socat > /dev/null || (echo "ERROR: socat not found. Install with:" && echo "  Ubuntu/Debian: sudo apt-get install socat" && echo "  Fedora: sudo dnf install socat" && echo "  Arch: sudo pacman -S socat" && exit 1)
	@echo "✓ All dependencies found"

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(SRCDIR)/*.o

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	sudo mkdir -p /usr/local/share/pixmaps
	sudo cp bridge-icon.png /usr/local/share/pixmaps/ 2>/dev/null || true
	sudo cp bridge-icon.jpg /usr/local/share/pixmaps/ 2>/dev/null || true
	@echo "Installed to /usr/local/bin/$(TARGET)"
	@echo "Icons installed to /usr/local/share/pixmaps/"

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo rm -f /usr/local/share/pixmaps/bridge-icon.png
	sudo rm -f /usr/local/share/pixmaps/bridge-icon.jpg
	@echo "Uninstalled $(TARGET)"

run: $(TARGET)
	./$(TARGET)

help:
	@echo "BRIDGE - Virtual Null Modem Bridge Build System"
	@echo "=============================================="
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the virtual null modem bridge (default)"
	@echo "  clean      - Remove built files"
	@echo "  install    - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall  - Remove from /usr/local/bin (requires sudo)"
	@echo "  run        - Build and run the bridge"
	@echo "  check-deps - Check if all dependencies are installed"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Features:"
	@echo "  ✓ Virtual null modem creation using socat"
	@echo "  ✓ GTK3 graphical interface"
	@echo "  ✓ Configurable device paths"
	@echo "  ✓ Communication testing"
	@echo "  ✓ Real-time status monitoring"
	@echo "  ✓ Comprehensive logging"
	@echo "  ✓ Settings persistence"
	@echo "  ✓ Compatible with LAST serial terminal"
	@echo ""
	@echo "Architecture:"
	@echo "  - Modular design with separate source files for each functional area"
	@echo "  - src/main.c - application initialization and coordination"
	@echo "  - src/nullmodem.c - socat process management and device operations"
	@echo "  - src/ui.c - GTK user interface creation"
	@echo "  - src/utils.c - utility functions (logging, formatting, timers)"
	@echo "  - src/callbacks.c - GTK event handlers"
	@echo "  - src/settings.c - configuration persistence"
	@echo "  - Event-driven GUI with responsive interface"
	@echo "  - Threaded monitoring for process health"
	@echo ""
	@echo "Dependencies:"
	@echo "  - GTK3 development libraries"
	@echo "  - pthread library"
	@echo "  - socat utility"
	@echo "  - Standard C development tools"
