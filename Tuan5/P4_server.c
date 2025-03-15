#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[50];
} Client;

Client clients[MAX_CLIENTS];

void broadcast_message(int sender_fd, char *message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && clients[i].socket != sender_fd) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server đang chạy trên cổng %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (client_fd < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            char username[50];
            recv(client_fd, username, sizeof(username), 0);
            username[strcspn(username, "\n")] = 0;

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = client_fd;
                    strcpy(clients[i].username, username);
                    break;
                }
            }

            printf("%s đã tham gia phòng chat.\n", username);
            sprintf(buffer, "%s đã tham gia phòng chat.\n", username);
            broadcast_message(client_fd, buffer);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (FD_ISSET(sd, &readfds)) {
                int bytes_read = read(sd, buffer, BUFFER_SIZE);
                if (bytes_read == 0) {
                    printf("%s đã rời khỏi phòng chat.\n", clients[i].username);
                    sprintf(buffer, "%s đã rời khỏi phòng chat.\n", clients[i].username);
                    broadcast_message(sd, buffer);
                    close(sd);
                    clients[i].socket = 0;
                } else {
                    buffer[bytes_read] = '\0';
                    char msg[BUFFER_SIZE + 50];
                    sprintf(msg, "%s: %s", clients[i].username, buffer);
                    broadcast_message(sd, msg);
                }
            }
        }
    }

    return 0;
}
