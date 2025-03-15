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

typedef struct {
    int socket;
    char username[50];
} Client;

Client clients[MAX_CLIENTS] = {0};

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server đang lắng nghe trên cổng %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("Select error");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (client_fd < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            char username[50];
            recv(client_fd, username, 50, 0);
            username[strcspn(username, "\n")] = 0;

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = client_fd;
                    strncpy(clients[i].username, username, 50);
                    printf("%s đã tham gia phòng chat.\n", username);
                    snprintf(buffer, BUFFER_SIZE, "%s đã tham gia phòng chat.\n", username);
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].socket != 0)
                            send(clients[j].socket, buffer, strlen(buffer), 0);
                    }
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (FD_ISSET(sd, &readfds)) {
                int bytes_read = read(sd, buffer, BUFFER_SIZE);
                if (bytes_read == 0) {
                    printf("%s đã rời phòng chat.\n", clients[i].username);
                    snprintf(buffer, BUFFER_SIZE, "%s đã rời phòng chat.\n", clients[i].username);
                    close(sd);
                    clients[i].socket = 0;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].socket != 0)
                            send(clients[j].socket, buffer, strlen(buffer), 0);
                    }
                } else {
                    buffer[bytes_read] = '\0';
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].socket != 0 && j != i) {
                            send(clients[j].socket, clients[i].username, strlen(clients[i].username), 0);
                            send(clients[j].socket, ": ", 2, 0);
                            send(clients[j].socket, buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
