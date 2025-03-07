#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 1024
#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char recvline[MAXLINE];
    ssize_t n;
    char server_ip_str[INET_ADDRSTRLEN];
    unsigned char binary_ip_buffer[sizeof(struct in_addr)];

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    printf("Enter server IP address: ");
    if (fgets(server_ip_str, INET_ADDRSTRLEN, stdin) == NULL) {
        perror("fgets for server IP failed");
        exit(EXIT_FAILURE);
    }
    server_ip_str[strcspn(server_ip_str, "\n")] = 0;

    // Convert IPv4 address from text to binary form and set it
    if (inet_pton(AF_INET, server_ip_str, &servaddr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    memcpy(binary_ip_buffer, &servaddr.sin_addr, sizeof(struct in_addr));

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send binary IP to the server
    printf("Sending binary IP to server...\n");
    write(sockfd, binary_ip_buffer, sizeof(struct in_addr));

    // Read the server's response
    if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = '\0';  // Null-terminate the received string
        printf("Server response: %s", recvline);
    } else {
        perror("Read failed");
    }

    close(sockfd);
    return 0;
}