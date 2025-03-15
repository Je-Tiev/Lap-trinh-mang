#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int server_fd;
int client_sockets[MAX_CLIENTS] = {0};
volatile sig_atomic_t stop_server = 0;

void signal_handler(int signo) {
    stop_server = 1;
}

void broadcast_message(int sender_fd, int *client_sockets, int num_clients, char *message) {
    for (int i = 0; i < num_clients; i++) {
        int client_fd = client_sockets[i];
        if (client_fd != sender_fd && client_fd > 0) {
            send(client_fd, message, strlen(message), 0);
        }
    }
}

int main() {
    struct sockaddr_in address;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

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

    while (!stop_server) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        struct timespec timeout = {5, 0};
        sigset_t empty_mask;
        sigemptyset(&empty_mask);

        int activity = pselect(max_sd + 1, &readfds, NULL, NULL, &timeout, &empty_mask);
        if (activity < 0 && errno != EINTR) {
            perror("pselect error");
            break;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            int client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (client_fd < 0) {
                perror("Accept failed");
                continue;
            }

            printf("Client mới kết nối: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    printf("Thêm client vào danh sách ở vị trí %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                int bytes_read = read(sd, buffer, BUFFER_SIZE);
                if (bytes_read == 0) {
                    printf("Client %d đã ngắt kết nối\n", i);
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buffer[bytes_read] = '\0';
                    printf("Tin nhắn từ client %d: %s", i, buffer);
                    broadcast_message(sd, client_sockets, MAX_CLIENTS, buffer);
                }
            }
        }
    }

    printf("Đang đóng server...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] > 0) {
            close(client_sockets[i]);
        }
    }
    close(server_fd);

    printf("Server đã dừng.\n");
    return 0;
}
