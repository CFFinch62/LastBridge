# Makefile for BRIDGE Examples

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -g

TARGETS = sniff_client

.PHONY: all clean help

all: $(TARGETS)

sniff_client: sniff_client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

help:
	@echo "BRIDGE Examples"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build all examples"
	@echo "  sniff_client - Build the sniffing client example"
	@echo "  clean        - Remove built examples"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Usage examples:"
	@echo "  ./sniff_client pipe  - Read from named pipe"
	@echo "  ./sniff_client tcp   - Connect to TCP server"
	@echo "  ./sniff_client udp   - Listen for UDP packets"
