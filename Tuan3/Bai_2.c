#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    char ip_str[16];
    struct in_addr ip_addr;
    
    printf("Nhập địa chỉ IP: ");
    scanf("%15s", ip_str);

    if (inet_pton(AF_INET, ip_str, &ip_addr) == 1) {
        printf("inet_pton: Chuyển đổi địa chỉ IP thành công: %s\n", ip_str);
    } else {
        printf("inet_pton: Chuyển đổi địa chỉ IP thất bại: %s\n", ip_str);
        exit(EXIT_FAILURE);
    }

    char ip_str_converted[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &ip_addr, ip_str_converted, INET_ADDRSTRLEN) != NULL) {
        printf("inet_ntop: Địa chỉ IP sau khi chuyển đổi lại: %s\n", ip_str_converted);
    } else {
        printf("inet_ntop: Chuyển đổi địa chỉ IP về lại chuỗi thất bại\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}