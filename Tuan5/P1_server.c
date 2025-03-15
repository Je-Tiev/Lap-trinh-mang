#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    int max_sd, activity;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Clear the socket set
        FD_ZERO(&read_fds);

        // Add server socket to set
        FD_SET(server_fd, &read_fds);
        max_sd = server_fd;

        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            // If valid socket descriptor then add to read list
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &read_fds);
                // Keep track of the maximum socket descriptor
                if (client_sockets[i] > max_sd) max_sd = client_sockets[i];
            }
        }

        // Wait for activity on one of the sockets
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        
        if (activity < 0) {
            perror("Select error");
            continue;
        }

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(server_fd, &read_fds)) {
            client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
            if (client_fd < 0) {
                perror("Accept failed");
                continue;
            }
            printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++) {
                // If position is empty
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    break;
                }
            }
        }

        // Check for IO operations on other sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &read_fds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                // Client disconnected
                if (valread <= 0) {
                    printf("Client disconnected.\n");

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    // Process the incoming message
                    buffer[valread] = '\0';
                    printf("Client: %s", buffer);
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] > 0 && client_sockets[j] != sd) {
                            // Optionally, send a response back to the client
                            send(client_sockets[j], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
