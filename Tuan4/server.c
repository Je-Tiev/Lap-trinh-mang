#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 8080 // Using PORT 8080 as in concurrent_server.c example
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5 // Maximum number of concurrent clients
#define NUM_QUESTIONS 10

// Structure to hold question data
typedef struct {
    char question[256];
    char options[4][100];
    char correct_answer;
} Question;

// Function prototypes
void sigchld_handler(int sig);
void handle_client(int connfd);
void send_question(int client_socket, const Question *q);
char receive_answer(int client_socket);
void send_feedback(int client_socket, const char *feedback);
void send_score(int client_socket, int score);

// Signal handler to prevent zombie processes
void sigchld_handler(int sig) {
    (void)sig; // Ignore unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int connfd) {
    int score = 0;
    Question questions[NUM_QUESTIONS] = {
        {"Câu hỏi 1: Thủ đô của Việt Nam là gì?", {"A. Hà Nội", "B. TP. Hồ Chí Minh", "C. Đà Nẵng", "D. Hải Phòng"}, 'A'},
        {"Câu hỏi 2: Sông nào dài nhất Việt Nam?", {"A. Sông Hồng", "B. Sông Cửu Long", "C. Sông Đà", "D. Sông Mã"}, 'B'},
        {"Câu hỏi 3: Địa chỉ ĐHBKHN?", {"A. Số 1 Tôn Thất Tùng", "B. Số 1 Giải Phóng", "C. Số 1 Trường Chinh", "D. Số 1 Đại Cồ Việt"}, 'C'},
        {"Câu hỏi 4: CPA đạt loại xuất sắc?", {"A. 2.4", "B. 2.8", "C. 3.2", "D. 3.6"}, 'D'},
        {"Câu hỏi 5: Điểm IELTS cao nhất là bao nhiêu?", {"A. 10", "B. 9.5", "C. 9.0", "D. 8.5"}, 'B'},
        {"Câu hỏi 6: TOEIC bao nhiêu thì không bị giới hạn tín chỉ?", {"A. 500", "B. 450 2", "C. 400", "D. 350"}, 'B'},
        {"Câu hỏi 7: Thư viện TQB có bao nhiêu tầng?", {"A. 12", "B. 11", "C. 10", "D. 9"}, 'C'},
        {"Câu hỏi 8: Đâu là nóc nhà Đông Dương", {"A. Núi Phú Sĩ", "B. Dãy Trường Sơn", "C. Núi Bà đen", "D. Fansipan"}, 'D'},
        {"Câu hỏi 9: B1 có bao nhiêu tầng?", {"A. 10", "B. 9", "C. 8", "D. 7"}, 'A'},
        {"Câu hỏi 10: Ngày Quốc khánh Việt Nam là ngày nào?", {"A. 30/4", "B. 1/5", "C. 2/9", "D. 19/5"}, 'C'}
    };

    int question_indices[NUM_QUESTIONS];
    for (int i = 0; i < NUM_QUESTIONS; ++i) {
        question_indices[i] = i;
    }
    srand(time(NULL) ^ getpid());

    for (int i = NUM_QUESTIONS - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        int temp = question_indices[i];
        question_indices[i] = question_indices[j];
        question_indices[j] = temp;
    }

    for (int i = 0; i < NUM_QUESTIONS; ++i) {
        send_question(connfd, &questions[question_indices[i]]);
        char answer = receive_answer(connfd);
        if (answer == questions[question_indices[i]].correct_answer) {
            score++;
            send_feedback(connfd, "Đúng rồi!\n");
        } else {
            char feedback_msg[BUFFER_SIZE];
            sprintf(feedback_msg, "Sai rồi. Đáp án đúng là %c\n", questions[question_indices[i]].correct_answer);
            send_feedback(connfd, feedback_msg);
        }
    }

    send_score(connfd, score);
    close(connfd);
}


void send_question(int client_socket, const Question *q) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    sprintf(buffer, "%s\n", q->question);
    for (int i = 0; i < 4; ++i) {
        strcat(buffer, q->options[i]);
        strcat(buffer, "\n");
    }

    if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("Lỗi gửi câu hỏi");
        exit(EXIT_FAILURE);
    }
}


char receive_answer(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Client disconnected while waiting for answer.\n");
            exit(EXIT_SUCCESS); 
        } else {
            perror("Lỗi nhận câu trả lời"); 
            exit(EXIT_FAILURE);
        }
    }
    return buffer[0];
}


void send_feedback(int client_socket, const char *feedback) {
    if (send(client_socket, feedback, strlen(feedback), 0) < 0) {
        perror("Lỗi gửi phản hồi"); 
        exit(EXIT_FAILURE);
    }
}


void send_score(int client_socket, int score) {
    char score_msg[BUFFER_SIZE];
    sprintf(score_msg, "Bài kiểm tra kết thúc. Điểm của bạn là: %d/%d\n", score, NUM_QUESTIONS);
    if (send(client_socket, score_msg, strlen(score_msg), 0) < 0) {
        perror("Lỗi gửi điểm");
        exit(EXIT_FAILURE);
    }
}


int main() {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pid_t pid;

    // Seed for random number generation for initial shuffle in main (less important in concurrent setup, but good practice)
    srand(time(NULL));


    // Create the listening socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    // Bind the listening socket to the specified port
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);

    printf("Server is listening on port %d...\n", PORT);

    // Server loop to accept multiple clients
    while (1) {
        // Accept an incoming connection
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
        } else if (pid == 0) {
            // Child process: handle the client
            printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            close(listenfd);  // Close the listening socket in the child process
            handle_client(connfd);
            exit(0); // Child process exits after handling client in handle_client function
        } else {
            // Parent process: continue accepting new clients
            close(connfd);  // Close the client socket in the parent process
        }
    }

    close(listenfd); // Should not reach here in normal operation
    return 0;
}