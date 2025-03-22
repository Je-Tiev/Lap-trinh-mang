// udp_client.c (Cải tiến)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024
#define TIMEOUT 5 // Timeout trong 5 giây
#define KEY 0x5A  // Khóa XOR Encryption

// Hàm mã hóa/giải mã bằng XOR
void xor_encrypt_decrypt(char *data, int length) {
    for (int i = 0; i < length; i++) {
        data[i] ^= KEY;
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, recv_servaddr;
    socklen_t len = sizeof(recv_servaddr);

    // Tạo socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình timeout cho socket
    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Cấu hình server
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "192.168.2.118", &servaddr.sin_addr);

    // Nhập dữ liệu từ người dùng
    printf("Enter message: ");
    fgets(buffer, MAXLINE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    // Mã hóa dữ liệu trước khi gửi
    xor_encrypt_decrypt(buffer, strlen(buffer));

    // Gửi dữ liệu
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Message sent to server (%s:%d)\n", "127.0.0.1", PORT);

    // Nhận phản hồi từ server
    int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&recv_servaddr, &len);
    if (n < 0) {
        perror("Timeout or receive failed");
        close(sockfd);
        return 1;
    }

    buffer[n] = '\0'; // Null terminate

    // Xác minh server bằng memcmp()
    if (memcmp(&servaddr, &recv_servaddr, sizeof(struct sockaddr_in)) == 0) {
        xor_encrypt_decrypt(buffer, strlen(buffer)); // Giải mã phản hồi
        printf("Server verified. Received: %s\n", buffer);
    } else {
        printf("Warning: Response from unknown server!\n");
    }

    close(sockfd);
    return 0;
}
