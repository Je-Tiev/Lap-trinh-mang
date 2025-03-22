#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAXLINE 1024
#define SERVER_IP "127.0.0.1" // Địa chỉ IP máy chủ, thay đổi nếu cần
#define XOR_KEY 'K'
#define TIMEOUT_SEC 5

void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *message = "Xin chao server, day la client.";
    struct sockaddr_in servaddr, expected_servaddr, received_servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Tao socket that bai");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&expected_servaddr, 0, sizeof(expected_servaddr));
    memset(&received_servaddr, 0, sizeof(received_servaddr));

    expected_servaddr.sin_family = AF_INET;
    expected_servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &expected_servaddr.sin_addr) <= 0) {
        perror("Dia chi IP server khong hop le");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    memcpy(&servaddr, &expected_servaddr, sizeof(expected_servaddr));

    if (connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Ket noi den server that bai");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Da ket noi den server.\n");

    char encrypted_message_initial[MAXLINE];
    strcpy(encrypted_message_initial, message);
    xor_cipher(encrypted_message_initial, XOR_KEY);

    if (send(sockfd, encrypted_message_initial, strlen(encrypted_message_initial), 0) < 0) {
        perror("Gui message ban dau den server that bai");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Da gui message ma hoa ban dau den server: %s\n", encrypted_message_initial);

    int n, server_len = sizeof(received_servaddr);
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    int retval = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

    if (retval == -1) {
        perror("select() that bai");
        close(sockfd);
        exit(EXIT_FAILURE);
    } else if (retval == 0) {
        printf("Timeout khi nhan message tu server.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    } else {
        if (FD_ISSET(sockfd, &readfds)) {
            n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&received_servaddr, (socklen_t *)&server_len);
            if (n < 0) {
                perror("recvfrom() that bai");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            buffer[n] = '\0';
            printf("Da nhan message ma hoa tu server: %s\n", buffer);

            if (memcmp(&expected_servaddr.sin_addr, &received_servaddr.sin_addr, sizeof(struct in_addr)) != 0 ||
                memcmp(&expected_servaddr.sin_port, &received_servaddr.sin_port, sizeof(in_port_t)) != 0) {
                fprintf(stderr, "Canh bao: Dia chi server khong khop voi dia chi mong doi!\n");
                char received_ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &received_servaddr.sin_addr, received_ip_str, INET_ADDRSTRLEN);
                fprintf(stderr, "Dia chi IP nhan duoc: %s, Port: %d\n", received_ip_str, ntohs(received_servaddr.sin_port));
                char expected_ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &expected_servaddr.sin_addr, expected_ip_str, INET_ADDRSTRLEN);
                fprintf(stderr, "Dia chi IP mong doi: %s, Port: %d\n", expected_ip_str, ntohs(expected_servaddr.sin_port));
            } else {
                printf("Dia chi server da duoc xac minh thanh cong.\n");
            }

            xor_cipher(buffer, XOR_KEY);
            printf("Message da giai ma tu server: %s\n", buffer);

            char *response_message = "Day la phan hoi tu client.";
            char encrypted_response[MAXLINE];
            strcpy(encrypted_response, response_message);
            xor_cipher(encrypted_response, XOR_KEY);

            if (sendto(sockfd, encrypted_response, strlen(encrypted_response), 0, (const struct sockaddr *)&received_servaddr, sizeof(received_servaddr)) < 0) {
                perror("Gui phan hoi da ma hoa den server that bai");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            printf("Da gui phan hoi da ma hoa den server: %s\n", encrypted_response);

        }
    }

    close(sockfd);
    return 0;
}
