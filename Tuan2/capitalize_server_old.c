#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h> // For toupper()

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to capitalize a string in-place
void capitalize_string(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = toupper(str[i]);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept a connection from the client
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Read data sent by the client
    int bytes_received = read(new_socket, buffer, BUFFER_SIZE);
    if (bytes_received < 0) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    } else if (bytes_received == 0) {
        printf("Client disconnected.\n");
    } else {
        printf("Message from client: %s\n", buffer);

        // Capitalize the received string
        capitalize_string(buffer);
        printf("Capitalized message: %s\n", buffer);

        // Send the capitalized string back to the client
        if (send(new_socket, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }
        printf("Capitalized message sent to client\n");
    }


    // Close the sockets
    close(new_socket);
    close(server_fd);

    return 0;
}
