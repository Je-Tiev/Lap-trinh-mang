#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h> 
#include <errno.h>  

#define PORT 8080
#define MAXLINE 1024
#define XOR_KEY 'K' // Khóa XOR
#define TIMEOUT_SEC 5 // Thời gian chờ timeout (giây)
#define NUM_MESSAGES 3

void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    char *messages[NUM_MESSAGES] = {
        "Ban la client thu nhat.",
        "Chao mung client thu hai!",
        "Client thu ba, rat vui duoc gap ban."
    };
    int message_index = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Tao socket that bai");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Lien ket that bai");
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(cliaddr); 
    int n;
    fd_set readfds;
    struct timeval timeout;

    printf("Server dang chay...\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int retval = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (retval == -1) {
            perror("select()");
            continue;
        } else if (retval == 0) {
            printf("Timeout cho recvfrom()... Cho doi client tiep theo.\n");
            continue;
        } else {
            if (FD_ISSET(sockfd, &readfds)) {
                n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
                if (n < 0) {
                    perror("recvfrom() that bai");
                    continue;
                }
                buffer[n] = '\0';
                printf("Da nhan message ma hoa tu client: %s\n", buffer);

                xor_cipher(buffer, XOR_KEY);
                printf("Message da giai ma tu client: %s\n", buffer);

                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, INET_ADDRSTRLEN);
                printf("Client IP: %s, Port: %d\n", client_ip, ntohs(cliaddr.sin_port));

                char *message_to_send = messages[message_index % NUM_MESSAGES];
                message_index++;
                char encrypted_message[MAXLINE];
                strcpy(encrypted_message, message_to_send);
                xor_cipher(encrypted_message, XOR_KEY);

                sendto(sockfd, (const char *)encrypted_message, strlen(encrypted_message), 0, (const struct sockaddr *)&cliaddr, len);
                printf("Da gui message ma hoa den client: %s\n", encrypted_message);

                FD_ZERO(&readfds);
                FD_SET(sockfd, &readfds);
                timeout.tv_sec = TIMEOUT_SEC;
                timeout.tv_usec = 0;
                retval = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

                if (retval > 0 && FD_ISSET(sockfd, &readfds)) {
                    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
                    if (n < 0) {
                        perror("recvfrom() acknowledgement that bai");
                        continue;
                    }
                    buffer[n] = '\0';
                    printf("Da nhan acknowledgement ma hoa tu client: %s\n", buffer);
                    xor_cipher(buffer, XOR_KEY);
                    printf("Acknowledgement da giai ma tu client: %s\n", buffer);
                    if (strcmp(buffer, "ACK") == 0) {
                        printf("Da nhan ACK hop le tu client.\n");
                    } else {
                        printf("Acknowledgement khong hop le tu client.\n");
                    }
                } else {
                    printf("Timeout cho acknowledgement tu client.\n");
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
