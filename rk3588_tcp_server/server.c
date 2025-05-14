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
#define SERVER_PORT 8080
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
int main() 
{
    print_device_ip();
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 创建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 配置服务器地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 绑定套接字到地址
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听连接请求
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", SERVER_PORT);
    
    
    // 接受客户端连接
    while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) >= 0) {
        printf("New client connected: %s ,port: %d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        
        pthread_t send_thread, receive_thread;
        // 创建发送线程
        if (pthread_create(&send_thread, NULL, send_message, &client_fd) != 0) {
            perror("pthread_create send_thread");
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        // 创建接收线程
        if (pthread_create(&receive_thread, NULL, receive_message, &client_fd) != 0) {
            perror("pthread_create receive_thread");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
        
        pthread_join(send_thread, NULL);
        pthread_join(receive_thread, NULL);
    }

    close(server_fd);
    return 0;
}