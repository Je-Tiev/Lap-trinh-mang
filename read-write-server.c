#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAXLINE 1024
#define PORT 8080
#define MAX_ADDR_STRLEN 128 // Define MAX_ADDR_STRLEN here as well

ssize_t readline(int fd, void *vptr, size_t maxlen);

// Custom sock_ntop function to convert a socket address into a string (IP and port)
char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    static char str[MAX_ADDR_STRLEN];   // Buffer to hold the string representation
    char portstr[8];                    // Buffer to hold the port as a string

    // Check if the address is IPv4
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    }
    // Check if the address is IPv6
    else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin6->sin6_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    } else {
        return NULL;  // Unsupported address family
    }
}


int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    unsigned char binary_ip_buffer[sizeof(struct in_addr)]; // Buffer to receive binary IP
    char client_ip_str_received[MAX_ADDR_STRLEN]; // Buffer for string IP from binary - use MAX_ADDR_STRLEN
    char response_message[MAXLINE];
    ssize_t n;
    const char *client_ip_readable; // Pointer to the string IP from sock_ntop

    // Create a listening socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connection
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    if (connfd < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Read binary IP from client
    printf("Waiting to receive binary IP from client...\n");
    if ((n = read(connfd, binary_ip_buffer, sizeof(struct in_addr))) != sizeof(struct in_addr)) {
        perror("Read binary IP failed or wrong size");
        close(connfd);
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    printf("Binary IP received from client.\n");

    struct in_addr received_in_addr;
    memcpy(&received_in_addr, binary_ip_buffer, sizeof(struct in_addr)); // Copy buffer to in_addr struct

    // Convert binary IP back to string using sock_ntop
    client_ip_readable = sock_ntop((const struct sockaddr *)&cliaddr, clilen); // Use cliaddr and clilen directly

    if (client_ip_readable == NULL) {
        perror("sock_ntop failed to convert binary IP to string");
        strcpy(client_ip_str_received, "Conversion Failed");
    } else {
        strncpy(client_ip_str_received, client_ip_readable, MAX_ADDR_STRLEN - 1);
        client_ip_str_received[MAX_ADDR_STRLEN - 1] = '\0'; // Ensure null termination
    }

    printf("Converted binary IP to string using sock_ntop: %s\n", client_ip_str_received);

    // Prepare response message with the converted IP string
    snprintf(response_message, MAXLINE, "Server received and converted your IP using sock_ntop: %s\n", client_ip_str_received);

    // Send the string IP back to the client
    printf("Sending string IP back to client: %s", response_message);
    if (write(connfd, response_message, strlen(response_message)) < 0) {
        perror("Write response failed");
        exit(EXIT_FAILURE);
    }
    printf("String IP sent to client.\n");


    close(connfd);
    close(listenfd);
    return 0;
}

// Function to read a line (up to \n) from a descriptor (not used in this version directly for client message)
ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break;  // Stop at newline
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0; // No data read
            } else {
                break;  // Some data was read
            }
        } else {
            return -1; // Error in read
        }
    }
    *ptr = 0;
    return n;
}
