#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
void print_device_ip()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];  //NI_MAXHOST 是一个未定义的标识符。NI_MAXHOST 是一个常量，通常定义在 <netdb.h> 头文件中，用于指定 getnameinfo 函数中主机名的最大长度。

    // 获取网络接口地址列表
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // 遍历所有网络接口
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // 检查是否为IPv4地址
        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                continue;
            }
            // 打印接口名称和IP地址
            printf("Interface: %s, IP Address: %s\n", ifa->ifa_name, host);
        }
    }
    freeifaddrs(ifaddr);
}


void *send_message(void *arg)
{
    int sock = *(int *)arg;
    char send_buffer[BUFFER_SIZE] = {0};
    printf("Enter message to send to server (type 'exit' to quit): \r\n");

    while (1)
    {
        fgets(send_buffer, sizeof(send_buffer), stdin);
        send_buffer[strlen(send_buffer) - 1] = '\0';
        // 如果用户输入了 "exit"，则退出循环
        if (!strcmp(send_buffer, "exit") || !strcmp(send_buffer, "quit")) {
            close(sock);
            pthread_exit(NULL);
        }

        if (send(sock, send_buffer, strlen(send_buffer), 0) <= 0) {
            perror("Send failed");
        }
        printf("Send: {%s}\n", send_buffer);
        memset(send_buffer, 0, sizeof(send_buffer));
    }

    pthread_exit(NULL);
}

void *receive_message(void *arg)
{
    int sock = *(int *)arg;
    char recv_buffer[BUFFER_SIZE] = {0};

    while (1)
    {
        // 接收服务器的响应
        if (recv(sock, recv_buffer, sizeof(recv_buffer), 0) <= 0) {
            perror("Recv failed");
            close(sock);
            pthread_exit(NULL);
        }
        printf("Receive: {%s}\n", recv_buffer);
        memset(recv_buffer, 0, sizeof(recv_buffer));
    }
    pthread_exit(NULL);
}


int main(void)
{
    char server_ip[INET_ADDRSTRLEN];
    int server_port;
    
    printf("\r\nPlease enter the IP and port of the server:\r\n");
    fflush(stdout);
    
    scanf("%s %d", server_ip, &server_port);
    // 打印设备的IP地址
    print_device_ip();
    
    int sock = 0;
    struct sockaddr_in serv_addr;

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // 将IP地址转换为二进制形式
    if(inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
    // 连接到服务器
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        sleep(1);
    }

    pthread_t send_thread, receive_thread;

    // 创建发送线程
    if (pthread_create(&send_thread, NULL, send_message, &sock) != 0) {
        perror("pthread_create send_thread");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 创建接收线程
    if (pthread_create(&receive_thread, NULL, receive_message, &sock) != 0) {
        perror("pthread_create receive_thread");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);
    
    close(sock);
    return 0;
}