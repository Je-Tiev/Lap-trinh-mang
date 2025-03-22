// udp_server.c (Cải tiến)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define KEY 0x5A // Khóa đơn giản cho XOR Encryption

struct client_data {
    struct sockaddr_in client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;
    int sockfd;
};

// Hàm mã hóa/giải mã dữ liệu bằng XOR
void xor_encrypt_decrypt(char *data, int length) {
    for (int i = 0; i < length; i++) {
        data[i] ^= KEY;
    }
}

// Xử lý client trong một thread riêng
void *handle_client(void *arg) {
    struct client_data *cdata = (struct client_data *)arg;

    // Giải mã thông điệp nhận được
    xor_encrypt_decrypt(cdata->buffer, strlen(cdata->buffer));
    printf("Received from client (%s:%d): %s\n",
           inet_ntoa(cdata->client_addr.sin_addr), ntohs(cdata->client_addr.sin_port), cdata->buffer);

    // Mã hóa phản hồi
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "Server ACK: %s", cdata->buffer);
    xor_encrypt_decrypt(response, strlen(response));

    // Gửi phản hồi đến client
    sendto(cdata->sockfd, response, strlen(response), 0,
           (struct sockaddr *)&cdata->client_addr, cdata->addr_len);

    free(cdata);
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, clientaddr;
    char buffer[BUFFER_SIZE];
    socklen_t len = sizeof(clientaddr);

    // Tạo socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ server
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Server listening on port %d...\n", PORT);

    while (1) {
        // Nhận dữ liệu từ client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientaddr, &len);
        if (n > 0) {
            buffer[n] = '\0'; // Null terminate

            // Tạo client_data để truyền vào thread
            struct client_data *cdata = malloc(sizeof(struct client_data));
            cdata->client_addr = clientaddr;
            cdata->addr_len = len;
            cdata->sockfd = sockfd;
            strcpy(cdata->buffer, buffer);

            // Tạo thread xử lý client
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, (void *)cdata);
            pthread_detach(tid);
        }
    }

    close(sockfd);
    return 0;
}
