#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    struct pollfd clients[MAX_CLIENTS + 1] = {0};
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    clients[0].fd = server_fd;
    clients[0].events = POLLIN;

    while (1) {
        poll(clients, MAX_CLIENTS + 1, -1);
        if (clients[0].revents & POLLIN) {
            client_fd = accept(server_fd, NULL, NULL);
            printf("Client mới kết nối!\n");
        }
    }
}
