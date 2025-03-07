#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080 // Make sure this matches the server port
#define BUFFER_SIZE 4096 // Increased buffer size to handle questions and options

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form.
    // Replace "127.0.0.1" with the actual IP of your server machine
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Đã kết nối đến máy chủ.\n"); // Connected to server message

    while (1) {
        memset(buffer, 0, sizeof(buffer)); // Clear buffer before receiving

        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Máy chủ đóng kết nối.\n"); // Server disconnected
            } else {
                perror("Lỗi nhận dữ liệu từ máy chủ"); // Error receiving data from server
            }
            break; // Connection closed or error
        }

        buffer[bytes_received] = '\0'; // Null-terminate received data
        printf("%s", buffer); // Print question/feedback from server

        if (strstr(buffer, "Điểm của bạn")) { // Check for final score message
            break; // Quiz finished, exit loop
        }

        char answer[BUFFER_SIZE];
        printf("Chọn đáp án (A, B, C, D): "); // Prompt for answer
        if (fgets(answer, BUFFER_SIZE, stdin) == NULL) {
            perror("Lỗi đọc đầu vào"); // Error reading input
            break; // Exit on input error (e.g., Ctrl+D)
        }
        answer[strcspn(answer, "\n")] = 0; // Remove trailing newline

        if (send(client_socket, answer, strlen(answer), 0) < 0) {
            perror("Lỗi gửi câu trả lời đến máy chủ"); // Error sending answer to server
            break; // Exit on send error
        }
    }

    // Close the socket
    close(client_socket);
    printf("Đóng kết nối.\n"); // Connection closed
    return 0;
}