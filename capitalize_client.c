#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE];

    // Creating socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    // Get input string from user
    printf("Enter a string: ");
    fgets(message, BUFFER_SIZE, stdin);
    // Remove newline character from fgets
    message[strcspn(message, "\n")] = 0;

    // Send the message to the server
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        return -1;
    }
    printf("Message sent to server: %s\n", message);

    // Read server's response
    int bytes_read = read(sock, buffer, BUFFER_SIZE);
    if (bytes_read < 0) {
        perror("Read failed");
        return -1;
    } else if (bytes_read == 0) {
        printf("Server disconnected.\n");
    } else {
        printf("Capitalized message from server: %s\n", buffer);
    }


    // Close the socket
    close(sock);

    return 0;
}
