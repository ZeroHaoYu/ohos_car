#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // 将IP地址转换为二进制形式
    if(inet_pton(AF_INET, "192.168.203.238", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // 连接到服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // 用户输入消息并发送给服务器
    printf("Enter message to send to server (type 'exit' to quit): ");
    while (1) {
        char user_message[1024] = {0};
        // 读取用户输入的消息
        fgets(user_message, sizeof(user_message), stdin);

        // 如果用户输入了 "exit"，则退出循环
        if (strcmp(user_message, "exit\n") == 0) {
            break;
        }

        // 发送用户消息到服务器
        if (send(sock, user_message, strlen(user_message), 0) < 0) {
            perror("Send failed");
            break;
        }
        printf("Message sent to server: %s", user_message);

        // 清空接收缓冲区
        memset(buffer, 0, sizeof(buffer));

        // 接收服务器的响应
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            perror("Recv failed");
            break;
        }
        printf("Server: %s\n", buffer);
    }

    // 关闭套接字
    close(sock);
    return 0;
}