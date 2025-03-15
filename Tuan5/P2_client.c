#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("192.168.2.118");

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Đã kết nối đến server. Nhập tin nhắn:\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        // Using select() to wait input 
        int activity = select(sock + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(sock, &readfds)) {
            int bytes_read = read(sock, buffer, BUFFER_SIZE);
            if (bytes_read == 0) {
                printf("Server đã đóng kết nối. Thoát...\n");
                break;
            }
            buffer[bytes_read] = '\0';
            printf("Tin nhắn từ server: %s", buffer);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            // Read input from the user
            fgets(buffer, BUFFER_SIZE, stdin);
            // Send the input to the server
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}
