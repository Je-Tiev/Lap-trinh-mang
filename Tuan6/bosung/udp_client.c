#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define XOR_KEY 0x42
#define TIMEOUT_SEC 10

int client_socket;

void xor_cipher(char *data, char key);

void handle_signal(int sig)
{
    if (client_socket > 0)
    {
        char exit_msg[] = "EXIT";
        xor_cipher(exit_msg, XOR_KEY);
        send(client_socket, exit_msg, strlen(exit_msg), 0);
        close(client_socket);
    }
    printf("\nĐã đóng kết nối.\n");
    exit(0);
}

void xor_cipher(char *data, char key)
{
    for (int i = 0; data[i] != '\0'; i++)
    {
        data[i] ^= key;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Sử dụng: %s <địa_chỉ_IP_server> <cổng_server> <tên_người_dùng>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    const char *username = argv[3];

    signal(SIGINT, handle_signal);

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Không thể tạo socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    struct sockaddr_in expected_server_addr = server_addr;

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Kết nối thất bại");
        close(client_socket);
        return 1;
    }

    printf("Đã kết nối đến server %s:%d\n", server_ip, server_port);

    char register_msg[BUFFER_SIZE];
    sprintf(register_msg, "REGISTER:%s", username);

    xor_cipher(register_msg, XOR_KEY);

    if (send(client_socket, register_msg, strlen(register_msg), 0) < 0)
    {
        perror("Gửi thất bại");
        close(client_socket);
        return 1;
    }

    fd_set read_fds;
    struct timeval timeout;

    char buffer[BUFFER_SIZE];
    char decrypted_buffer[BUFFER_SIZE];

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Fork thất bại");
        close(client_socket);
        return 1;
    }

    if (pid == 0)
    {
        while (1)
        {
            timeout.tv_sec = TIMEOUT_SEC;
            timeout.tv_usec = 0;

            FD_ZERO(&read_fds);
            FD_SET(client_socket, &read_fds);

            int activity = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);

            if (activity < 0)
            {
                perror("Select thất bại");
                break;
            }

            if (activity == 0)
            {
                continue;
            }

            if (FD_ISSET(client_socket, &read_fds))
            {
                struct sockaddr_in sender_addr;
                socklen_t sender_addr_len = sizeof(sender_addr);

                int bytes_received = recvfrom(client_socket, buffer, BUFFER_SIZE, 0,
                                              (struct sockaddr *)&sender_addr, &sender_addr_len);

                if (bytes_received > 0)
                {
                    buffer[bytes_received] = '\0';

                    if (memcmp(&sender_addr.sin_addr, &expected_server_addr.sin_addr,
                               sizeof(sender_addr.sin_addr)) != 0 ||
                        sender_addr.sin_port != expected_server_addr.sin_port)
                    {
                        printf("Cảnh báo: Nhận được tin nhắn từ nguồn không xác định!\n");
                        continue;
                    }

                    strcpy(decrypted_buffer, buffer);
                    xor_cipher(decrypted_buffer, XOR_KEY);

                    if (strcmp(decrypted_buffer, "SERVER_FULL") == 0)
                    {
                        printf("Server đã đầy, không thể kết nối.\n");
                        kill(getppid(), SIGINT);
                        break;
                    }
                    else if (strcmp(decrypted_buffer, "ACK") == 0)
                    {
                        continue;
                    }
                    else
                    {
                        printf("%s\n", decrypted_buffer);
                    }
                }
                else if (bytes_received == 0)
                {
                    printf("Server đã đóng kết nối.\n");
                    kill(getppid(), SIGINT);
                    break;
                }
                else
                {
                    perror("Nhận thất bại");
                }
            }
        }

        close(client_socket);
        exit(0);
    }
    else
    {
        while (1)
        {
            char input[BUFFER_SIZE];
            if (fgets(input, BUFFER_SIZE, stdin) == NULL)
            {
                break;
            }

            input[strcspn(input, "\n")] = '\0';

            if (strlen(input) == 0)
            {
                continue;
            }

            xor_cipher(input, XOR_KEY);

            if (send(client_socket, input, strlen(input), 0) < 0)
            {
                perror("Gửi thất bại");
                break;
            }

            char temp[BUFFER_SIZE];
            strcpy(temp, input);
            xor_cipher(temp, XOR_KEY);

            if (strcmp(temp, "EXIT") == 0)
            {
                printf("Đang thoát...\n");
                kill(pid, SIGTERM);
                break;
            }
        }
    }

    close(client_socket);
    return 0;
}
