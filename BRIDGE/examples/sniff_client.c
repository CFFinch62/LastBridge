/*
 * Example BRIDGE Sniffing Client
 * Demonstrates how to receive sniffed serial data from BRIDGE
 * 
 * This example shows three methods of receiving data:
 * 1. Named Pipe reader
 * 2. TCP client
 * 3. UDP listener
 * 
 * Compile: gcc -o sniff_client sniff_client.c
 * Usage: ./sniff_client [pipe|tcp|udp]
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define BUFFER_SIZE 4096
#define DEFAULT_PIPE_PATH "/tmp/bridge_sniff_pipe"
#define DEFAULT_TCP_PORT 8888
#define DEFAULT_UDP_PORT 9999
#define DEFAULT_UDP_ADDR "239.1.1.1"

volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
    printf("\nShutting down...\n");
}

int read_from_pipe(const char *pipe_path) {
    printf("Opening named pipe: %s\n", pipe_path);
    printf("Make sure BRIDGE is running with pipe output enabled.\n");
    printf("Press Ctrl+C to exit.\n\n");
    
    int fd = open(pipe_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open pipe");
        return 1;
    }
    
    char buffer[BUFFER_SIZE];
    while (running) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("PIPE: %s", buffer);
            fflush(stdout);
        } else if (bytes_read == 0) {
            // Pipe closed, try to reopen
            close(fd);
            usleep(100000); // 100ms
            fd = open(pipe_path, O_RDONLY);
            if (fd < 0) {
                perror("Failed to reopen pipe");
                break;
            }
        } else {
            if (errno != EAGAIN) {
                perror("Pipe read error");
                break;
            }
            usleep(10000); // 10ms
        }
    }
    
    close(fd);
    return 0;
}

int read_from_tcp(int port) {
    printf("Connecting to TCP server on localhost:%d\n", port);
    printf("Make sure BRIDGE is running with TCP output enabled.\n");
    printf("Press Ctrl+C to exit.\n\n");
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Failed to create TCP socket");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to TCP server");
        close(sock);
        return 1;
    }
    
    printf("Connected to BRIDGE TCP server!\n\n");
    
    char buffer[BUFFER_SIZE];
    while (running) {
        ssize_t bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("TCP: %s", buffer);
            fflush(stdout);
        } else if (bytes_read == 0) {
            printf("TCP connection closed by server\n");
            break;
        } else {
            perror("TCP receive error");
            break;
        }
    }
    
    close(sock);
    return 0;
}

int read_from_udp(int port, const char *addr) {
    printf("Listening for UDP packets on %s:%d\n", addr, port);
    printf("Make sure BRIDGE is running with UDP output enabled.\n");
    printf("Press Ctrl+C to exit.\n\n");
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Failed to create UDP socket");
        return 1;
    }
    
    // Enable address reuse
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("Failed to bind UDP socket");
        close(sock);
        return 1;
    }
    
    // Join multicast group if it's a multicast address
    if (inet_addr(addr) >= inet_addr("224.0.0.0") && inet_addr(addr) <= inet_addr("239.255.255.255")) {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(addr);
        mreq.imr_interface.s_addr = INADDR_ANY;
        
        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            perror("Failed to join multicast group");
            close(sock);
            return 1;
        }
        printf("Joined multicast group %s\n", addr);
    }
    
    printf("UDP listener ready!\n\n");
    
    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    
    while (running) {
        ssize_t bytes_read = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr*)&sender_addr, &sender_len);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("UDP: %s", buffer);
            fflush(stdout);
        } else if (bytes_read < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("UDP receive error");
                break;
            }
            usleep(10000); // 10ms
        }
    }
    
    close(sock);
    return 0;
}

void print_usage(const char *program_name) {
    printf("BRIDGE Sniffing Client Example\n");
    printf("Usage: %s [method]\n\n", program_name);
    printf("Methods:\n");
    printf("  pipe  - Read from named pipe (default: %s)\n", DEFAULT_PIPE_PATH);
    printf("  tcp   - Connect to TCP server (default: localhost:%d)\n", DEFAULT_TCP_PORT);
    printf("  udp   - Listen for UDP packets (default: %s:%d)\n", DEFAULT_UDP_ADDR, DEFAULT_UDP_PORT);
    printf("\nThis client demonstrates how to receive sniffed serial data from BRIDGE.\n");
    printf("Make sure BRIDGE is running with the corresponding output method enabled.\n");
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *method = argv[1];
    
    if (strcmp(method, "pipe") == 0) {
        return read_from_pipe(DEFAULT_PIPE_PATH);
    } else if (strcmp(method, "tcp") == 0) {
        return read_from_tcp(DEFAULT_TCP_PORT);
    } else if (strcmp(method, "udp") == 0) {
        return read_from_udp(DEFAULT_UDP_PORT, DEFAULT_UDP_ADDR);
    } else {
        printf("Unknown method: %s\n\n", method);
        print_usage(argv[0]);
        return 1;
    }
}
