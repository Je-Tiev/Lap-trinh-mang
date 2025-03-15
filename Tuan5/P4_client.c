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
    char buffer[BUFFER_SIZE], username[50];
    fd_set readfds;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("192.168.2.118");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Nhập tên của bạn: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    send(sock, username, strlen(username), 0);

    printf("Chào mừng %s! Bạn có thể bắt đầu chat.\n", username);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

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
            printf("%s", buffer);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            if (strncmp(buffer, "/exit", 5) == 0) {
                printf("Bạn đã thoát khỏi phòng chat.\n");
                break;
            }
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}
