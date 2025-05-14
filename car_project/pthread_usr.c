/**
  ******************************************************************************
  * @file           : pthread_usr.c
  * @author         : Dylan
  * @brief          : 线程函数实现
  * @attention      : 正在构建
  * @date           : 20203.3.23 构建了蓝牙串口数据收取 2023.3.22 构建了底盘串口通讯
  ******************************************************************************
  * *
  */

#include "pthread_usr.h"
/**
 * @brief  线程间通信工具，通过全局变量加一个锁实现   目前的数据流转和控制方向   BLE（read) -->BLE(Proc) -->通信管道更改，数据上锁 --> OUT(发送数据）-->解锁
 *          还需要实现的功能包括 IMU（read) -->IMU(Proc) --> 管道通信，数据上锁 --> OUT（判断锁的类型） -->OUT(发送数据） -->解锁
 */
PC_SendType send_data;
int fd_lora;
int fd_imu;
int FLAG;
int fd_chassis;
/**
 * @attention 蓝牙串口读取线程
 * @return
 */
void *_serial_BLE_input_task(void)
{
    int i = 0;
    int ret = ERR;
    int buf = 0;
    char recv[FRAME_LEN_BLE] = {0};
    pthread_detach(pthread_self());
    printf("test _ready\n");
    while (1)
    {
       // usleep(300000);
        for (i = 0; i < FRAME_LEN_BLE; i++)
        {
            ret = read(fd_BLE, &buf, 1);
            if (ret == ERR)
            {
                perror("read error\n");
                exit(0);
            }
            recv[i] = buf;
        }
        ret = data_proce_BLE(recv);
        //printf("char BUF is%c\n",ret);
            switch (ret) {
                case 'f':
                    send_data = ChangeSendType(100, 0, 0);
                    FLAG = 1;                                                         //对数据上锁
                    break;
                case 'b':
                    send_data = ChangeSendType(-100, 0, 0);
                    FLAG = 1;
                    break;
                case 'l':
                    send_data = ChangeSendType(0, 0, -200);
                    FLAG = 1;
                    break;
                case 'r':
                    send_data = ChangeSendType(0, 0, 200);
                    FLAG = 1;
                    break;
                default:
                    send_data = ChangeSendType(100, 0, 0);
                    FLAG = 1;
                    break;

            }
    }
}

/**
 * @attention 底盘控制输出线程
 * @return
 */


void _serial_output_task(void)
{

    pthread_detach(pthread_self());
    int ret;

    /**
     * 电机下电
     */

    //send_data=Power(0);
    //    write(fd_chassis, (unsigned char *) send_data.send_data, 12);
    //   usleep(100);

    /**
     *  电机上电
     */

    send_data=Power(1);
    write(fd_chassis, (char *) send_data.send_data, 12);

    while (1)
    {
        // printf("FLAG is %d",FLAG);

       send_data = ChangeSendType(-100, 0, 0);

       ret = write(fd_chassis, (char *) send_data.send_data, 12);

        if (ret > 0)
        {
            printf("send success\n");
        }
        else
        {
            printf("send error!\r\n");
        }
        FLAG=0;
        usleep(300000);
    }

}
float fAcc[3], fGyro[3], fAngle[3];
char imu_acc_buf[1000];
void _serial_IMU_input_task(void){

    int i;
    char cBuff[1];
    while(1)
    {

        while(serial_read_data(fd_imu, cBuff, 1))
        {
            WitSerialDataIn(cBuff[0]);
        }
        printf("\n");
        Delayms(500);

        if(s_cDataUpdate)
        {
            for(i = 0; i < 3; i++)
            {
                fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
                fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
                fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
            }

            if(s_cDataUpdate & ACC_UPDATE)
            {
                sprintf(imu_acc_buf,"acc:%.3f,%.3f,%.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
                printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
            }
            if(s_cDataUpdate & GYRO_UPDATE)
            {
                printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
            }
            if(s_cDataUpdate & ANGLE_UPDATE)
            {
                printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
            }
            if(s_cDataUpdate & MAG_UPDATE)
            {
                printf("mag:%d %d %d\r\n", sReg[HX], sReg[HY], sReg[HZ]);
            }
        }
    }

    serial_close(fd_imu);
}

void _serial_LORA_OUTPUT_task(void){
    while(1) {
        write(fd_lora, imu_acc_buf, strlen(imu_acc_buf));
        usleep(300000);
    }
}
