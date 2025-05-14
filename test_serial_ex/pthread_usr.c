/**
  ******************************************************************************
  * @file           : pthread_usr.c
  * @author         : Dylan
  * @brief          : 线程函数实现
  * @attention      : 正在构建
  * @date           : 2023.3.22
  ******************************************************************************
  */

#include "pthread_usr.h"

PC_SendType send_data;
/**
 * @attention 暂时未启用的串口读取函数
 * @return
 */
void *_serial_input_task(void)
{
    int i = 0;
    int ret = ERR;
    int buf = 0;
    char recv[FRAME_LEN] = {0};
    pthread_detach(pthread_self());
    printf("test _ready\n");
    while (1)
    {
        for (i = 0; i < FRAME_LEN; i++)
        {
            ret = read(fd_chassis, &buf, 1);
            if (ret == ERR)
            {
                perror("read error\n");
                exit(0);
            }
            recv[i] = buf;
        }
        ret = data_proce(recv);
        if (ret == ERR)
        {
            perror("data process error\n");
            exit(0);
        }
    }
}

void _serial_output_task(void)
{

    pthread_detach(pthread_self());
    int ret;


    //send_data=Power(0);
    //    write(fd_chassis, (unsigned char *) send_data.send_data, 12);
    //   usleep(100);
    send_data=Power(1);
    write(fd_chassis, (unsigned char *) send_data.send_data, 12);
    while (1)
    {

        send_data = ChangeSendType(0, 0, -200);

        ret = write(fd_chassis, (unsigned char *)send_data.send_data, 12);
        if (ret > 0)
        {
             printf("send success, data is  \n");
        }
        else
        {
            printf("send error!\r\n");
        }
        usleep(300000);
    }

}
