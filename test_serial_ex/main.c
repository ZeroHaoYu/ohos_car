/**
  ******************************************************************************
  * @file           : main.c
  * @author         : Dylan
  * @brief          : main入口函数
  * @attention      : 必须把串口启动放在main函数部分且不能封装，会乱码
  * @date           : 2023.3.22
  ******************************************************************************
  */

#include "main.h"
int fd_chassis;
int main(int argc, char **argv)
{
    char *uart_dev = UART_TTL_NAME;
    int ret = ERR;

    fd_chassis = open(uart_dev, O_RDWR);
    if ( fd_chassis == ERR)
    {
        perror("open file fail\n");
        return ERR;
    }
    ret = uart_init( fd_chassis, 115200L);
    if (ret == ERR)
    {
        perror("uart init error\n");
        return ERR;
    }

    pthread_t pid_t1;
   // pthread_t pid_t2;
    pthread_create(&pid_t1, NULL, (void *)_serial_output_task, 0);
    // pthread_create(&pid_t2, NULL, (void *)_serial_input_task, 0);
    while (1)
    {
        sleep(10L);
    }
    close(fd_chassis);
    return 0;
}
