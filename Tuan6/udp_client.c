#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define MAXLINE 1024
#define TIMEOUT_SEC 3  // Timeout 3 giây

int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *message = "Hello from client";
    struct sockaddr_in servaddr, recv_servaddr;
    socklen_t len = sizeof(recv_servaddr);
    char server_ip[] = "192.168.2.118";

    // Tạo socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập timeout cho socket
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Thiết lập thông tin server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Gửi tin nhắn tới server
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Message sent to server (%s:%d)\n", server_ip, PORT);

    // Nhận phản hồi từ server
    int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&recv_servaddr, &len);
    if (n < 0) {
        perror("Timeout or receive failed");
        close(sockfd);
        return 1;
    }

    buffer[n] = '\0';  // Kết thúc chuỗi

    // Xác minh server bằng memcmp()
    if (memcmp(&servaddr, &recv_servaddr, sizeof(struct sockaddr_in)) == 0) {
        printf("Verified server response from %s:%d\n", inet_ntoa(recv_servaddr.sin_addr), ntohs(recv_servaddr.sin_port));
        printf("Server message: %s\n", buffer);
    } else {
        printf("Warning: Response from unexpected server (%s:%d)\n", inet_ntoa(recv_servaddr.sin_addr), ntohs(recv_servaddr.sin_port));
    }

    close(sockfd);
    return 0;
}
