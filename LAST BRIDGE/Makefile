# Unified Makefile for Serial Communication Suite
# Builds both LAST (serial terminal) and BRIDGE (virtual null modem)

.PHONY: all clean install uninstall run-last run-bridge run-both check-deps help last bridge

# Default target builds both applications
all: check-deps last bridge

# Build individual applications
last:
	@echo "Building LAST - Linux Advanced Serial Transceiver..."
	$(MAKE) -C LAST

bridge:
	@echo "Building BRIDGE - Virtual Null Modem Bridge..."
	$(MAKE) -C BRIDGE

# Check dependencies for both applications
check-deps:
	@echo "Checking dependencies for Serial Communication Suite..."
	@echo "Checking LAST dependencies:"
	@$(MAKE) -C LAST check-deps
	@echo ""
	@echo "Checking BRIDGE dependencies:"
	@$(MAKE) -C BRIDGE check-deps
	@echo ""
	@echo "✓ All dependencies satisfied for both applications"

# Clean both applications
clean:
	@echo "Cleaning LAST..."
	$(MAKE) -C LAST clean
	@echo "Cleaning BRIDGE..."
	$(MAKE) -C BRIDGE clean
	@echo "✓ All build artifacts cleaned"

# Install both applications
install: all
	@echo "Installing LAST..."
	$(MAKE) -C LAST install
	@echo "Installing BRIDGE..."
	$(MAKE) -C BRIDGE install
	@echo "✓ Both applications installed to /usr/local/bin/"

# Uninstall both applications
uninstall:
	@echo "Uninstalling LAST..."
	$(MAKE) -C LAST uninstall
	@echo "Uninstalling BRIDGE..."
	$(MAKE) -C BRIDGE uninstall
	@echo "✓ Both applications uninstalled"

# Run individual applications
run-last: last
	@echo "Starting LAST - Linux Advanced Serial Transceiver..."
	cd LAST && ./last

run-bridge: bridge
	@echo "Starting BRIDGE - Virtual Null Modem Bridge..."
	cd BRIDGE && ./bridge

# Run both applications (BRIDGE first, then LAST)
run-both: all
	@echo "Starting Serial Communication Suite..."
	@echo "1. Starting BRIDGE (Virtual Null Modem)..."
	cd BRIDGE && ./bridge &
	@sleep 2
	@echo "2. Starting LAST (Serial Terminal)..."
	cd LAST && ./last

# Test the integration
test-integration: all
	@echo "Testing Serial Communication Suite Integration..."
	@echo "1. Starting BRIDGE in background..."
	cd BRIDGE && ./bridge &
	@BRIDGE_PID=$$!; \
	sleep 3; \
	echo "2. BRIDGE should be running (PID: $$BRIDGE_PID)"; \
	echo "3. You can now start LAST manually to test the integration"; \
	echo "4. Use Ctrl+C to stop BRIDGE when done testing"

help:
	@echo "Serial Communication Suite Build System"
	@echo "======================================"
	@echo ""
	@echo "This suite contains two applications:"
	@echo "  • LAST   - Linux Advanced Serial Transceiver (GTK3 serial terminal)"
	@echo "  • BRIDGE - Virtual Null Modem Bridge (creates virtual serial devices)"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build both LAST and BRIDGE (default)"
	@echo "  last             - Build only LAST"
	@echo "  bridge           - Build only BRIDGE"
	@echo "  clean            - Clean build artifacts for both applications"
	@echo "  install          - Install both applications to /usr/local/bin (requires sudo)"
	@echo "  uninstall        - Remove both applications from /usr/local/bin (requires sudo)"
	@echo "  run-last         - Build and run LAST"
	@echo "  run-bridge       - Build and run BRIDGE"
	@echo "  run-both         - Build and run both applications"
	@echo "  test-integration - Test BRIDGE and LAST integration"
	@echo "  check-deps       - Check if all dependencies are installed"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Usage Examples:"
	@echo "  make             # Build both applications"
	@echo "  make run-both    # Start both applications"
	@echo "  make install     # Install both (requires sudo)"
	@echo ""
	@echo "Integration Workflow:"
	@echo "  1. Run 'make run-bridge' to start the virtual null modem"
	@echo "  2. In BRIDGE, click 'Start Virtual Null Modem'"
	@echo "  3. Run 'make run-last' to start the serial terminal"
	@echo "  4. In LAST, refresh ports and connect to /tmp/ttyV0 or /tmp/ttyV1"
	@echo "  5. Test communication between the virtual devices"
	@echo ""
	@echo "Dependencies:"
	@echo "  • GTK3 development libraries (libgtk-3-dev)"
	@echo "  • pthread library"
	@echo "  • socat utility"
	@echo "  • Standard C development tools (gcc, make)"
	@echo ""
	@echo "For individual application help:"
	@echo "  make -C LAST help"
	@echo "  make -C BRIDGE help"
