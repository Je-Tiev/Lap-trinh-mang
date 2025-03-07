#include<stdio.h>
#include<netinet/in.h>
#include <arpa/inet.h>

int main(){
	struct sockaddr_in bai_1;
	char ip_str[INET_ADDRSTRLEN];
    int port;
    
    printf("Nhap dia chi IPv4: ");
    scanf("%15s", ip_str);
    
    printf("Nhap so cong (port): ");
    scanf("%d", &port);
    
	bai_1.sin_family = AF_INET;
    bai_1.sin_port = htons(port);
    inet_pton(AF_INET, ip_str, &bai_1.sin_addr);
	
	inet_ntop(AF_INET, &bai_1.sin_addr, ip_str, INET_ADDRSTRLEN);
	
    printf("\n  Cau truc sockaddr_in da duoc khoi tao:\n");
    printf("  Family      : AF_INET (IPv4)\n");
    printf("  Port        : %d\n", ntohs(bai_1.sin_port));
    printf("  IP Address  : %s\n", ip_str");
    
    return 0;
}