#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void broadcast_message(int sender_fd, struct pollfd *pfds, int num_clients, char *message) {
    for (int i = 1; i <= num_clients; i++) {
        if (pfds[i].fd > 0 && pfds[i].fd != sender_fd) {
            send(pfds[i].fd, message, strlen(message), 0);
        }
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    struct pollfd pfds[MAX_CLIENTS + 1];
    char buffer[BUFFER_SIZE];

    // Create a socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server đang lắng nghe trên cổng %d...\n", PORT);

    // Set up the pollfd structure for stdin
    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN;
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        pfds[i].fd = -1;
    }

    while (1) {
        int activity = poll(pfds, MAX_CLIENTS + 1, -1);
        if (activity < 0) {
            perror("Poll error");
            continue;
        }

        // Check connection
        if (pfds[0].revents & POLLIN) {
            socklen_t addrlen = sizeof(address);
            client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (client_fd < 0) {
                perror("Accept failed");
                continue;
            }

            printf("Client mới kết nối: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 1; i <= MAX_CLIENTS; i++) {
                if (pfds[i].fd < 0) {
                    pfds[i].fd = client_fd;
                    pfds[i].events = POLLIN;
                    break;
                }
            }
        }

        // Data from client
        for (int i = 1; i <= MAX_CLIENTS; i++) {
            if (pfds[i].fd > 0 && (pfds[i].revents & POLLIN)) {
                int bytes_read = read(pfds[i].fd, buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    printf("Client %d đã ngắt kết nối\n", i);
                    close(pfds[i].fd);
                    pfds[i].fd = -1;
                } else {
                    buffer[bytes_read] = '\0';
                    printf("Tin nhắn từ client %d: %s", i, buffer);
                    broadcast_message(pfds[i].fd, pfds, MAX_CLIENTS, buffer);
                }
            }
        }
    }

    return 0;
}
