# Makefile for LAST - Linux Advanced Serial Transceiver

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -g
TARGET = last

# GTK3 configuration
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

# Source files
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/serial.c $(SRCDIR)/network.c $(SRCDIR)/ui.c $(SRCDIR)/file_ops.c $(SRCDIR)/utils.c $(SRCDIR)/callbacks.c $(SRCDIR)/settings.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SRCDIR)/common.h $(SRCDIR)/serial.h $(SRCDIR)/network.h $(SRCDIR)/ui.h $(SRCDIR)/file_ops.h $(SRCDIR)/utils.h $(SRCDIR)/callbacks.h $(SRCDIR)/settings.h

.PHONY: all clean install uninstall run check-deps help

all: check-deps $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(GTK_LIBS) -lpthread

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -I$(SRCDIR) -c $< -o $@

check-deps:
	@echo "Checking dependencies..."
	@pkg-config --exists gtk+-3.0 || (echo "ERROR: GTK3 development libraries not found. Install with:" && echo "  Ubuntu/Debian: sudo apt-get install libgtk-3-dev" && echo "  Fedora: sudo dnf install gtk3-devel" && echo "  Arch: sudo pacman -S gtk3" && exit 1)
	@echo "✓ All dependencies found"

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(SRCDIR)/*.o

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	sudo mkdir -p /usr/local/share/pixmaps
	sudo cp last-icon.png /usr/local/share/pixmaps/ 2>/dev/null || true
	sudo cp last-icon.jpg /usr/local/share/pixmaps/ 2>/dev/null || true
	@echo "Installed to /usr/local/bin/$(TARGET)"
	@echo "Icons installed to /usr/local/share/pixmaps/"

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo rm -f /usr/local/share/pixmaps/last-icon.png
	sudo rm -f /usr/local/share/pixmaps/last-icon.jpg
	@echo "Uninstalled $(TARGET)"

run: $(TARGET)
	./$(TARGET)

help:
	@echo "LAST - Linux Advanced Serial Transceiver Build System"
	@echo "===================================="
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the complete serial terminal (default)"
	@echo "  clean      - Remove built files"
	@echo "  install    - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall  - Remove from /usr/local/bin (requires sudo)"
	@echo "  run        - Build and run the terminal"
	@echo "  check-deps - Check if all dependencies are installed"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Features:"
	@echo "  ✓ Full-featured GUI serial terminal"
	@echo "  ✓ Comprehensive port detection (overcomes Linux limitations)"
	@echo "  ✓ Finds virtual ports (PyVComm, /tmp/ttyV*, /dev/pts/*)"
	@echo "  ✓ Complete serial parameter control (data bits, parity, stop bits, flow control)"
	@echo "  ✓ Hex display mode"
	@echo "  ✓ Timestamps"
	@echo "  ✓ File operations (send file, save data)"
	@echo "  ✓ Data logging"
	@echo "  ✓ Control signals (DTR, RTS, Break)"
	@echo "  ✓ Statistics tracking"
	@echo "  ✓ Professional GUI layout"
	@echo ""
	@echo "Architecture:"
	@echo "  - Modular design with separate source files for each functional area"
	@echo "  - src/main.c - application initialization and coordination"
	@echo "  - src/serial.c - serial port operations and communication"
	@echo "  - src/ui.c - GTK user interface creation"
	@echo "  - src/file_ops.c - file operations (send, save, log)"
	@echo "  - src/utils.c - utility functions (formatting, timestamps, statistics)"
	@echo "  - src/callbacks.c - GTK event handlers"
	@echo "  - Event-driven GUI with responsive interface"
	@echo "  - Threaded I/O for non-blocking serial communication"
	@echo ""
	@echo "Dependencies:"
	@echo "  - GTK3 development libraries"
	@echo "  - pthread library"
	@echo "  - Standard C development tools"
