#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

//Broadcast function
void broadcast_message(int sender_fd, int *client_sockets, int num_clients, char *message) {
    for (int i = 0; i < num_clients; i++) {
        int client_fd = client_sockets[i];
        if (client_fd != sender_fd && client_fd > 0) {
            send(client_fd, message, strlen(message), 0);
        }
    }
}

int main() {
    int server_fd, client_fd, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server đang lắng nghe trên cổng %d...\n", PORT);

    while (1) {
        // Clear the socket set
        FD_ZERO(&readfds);
      
        //Add server socket to set
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            // Socket descriptor
            int sd = client_sockets[i];

            // If valid socket descriptor then add to read list
            if (sd > 0) FD_SET(sd, &readfds);

            // Keep track of the maximum socket descriptor
            if (sd > max_sd) max_sd = sd;
        }

        // Wait for activity on one of the sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("Select error");
        }

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("Accept failed");
                continue;
            }
            printf("Client mới kết nối: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    printf("Thêm client vào danh sách ở vị trí %d\n", i);
                    break;
                }
            }
        }

        // Check for IO operations on other sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                int bytes_read = read(sd, buffer, BUFFER_SIZE - 1);
                if (bytes_read <= 0) {
                    printf("Client %d đã ngắt kết nối\n", i);
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buffer[bytes_read] = '\0';
                    printf("Tin nhắn từ client %d: %s\n", i, buffer);
                    
                    // Broadcast message to all client
                    broadcast_message(sd, client_sockets, MAX_CLIENTS, buffer);
                }
            }
        }
    }

    return 0;
}
