/**
  ******************************************************************************
  * @file           : main.c
  * @author         : Dylan
  * @brief          : main入口函数
  * @attention      : 必须把串口启动放在main函数部分且不能封装，会乱码
  * @date           : 2023.3.23  2023.3.22
  ******************************************************************************
  */

#include "main.h"

int main(int argc, char **argv)
{
    int fd_imu;
    int fd_chassis;
    /**
     * @brief ：初始化底盘
     */
    char *uart_dev = UART_TTL_NAME;
    int ret = ERR;

    fd_chassis = open(uart_dev, O_RDWR);

    if ( fd_chassis == ERR)
    {
        perror("open chassis_file fail\n");
        return ERR;
    }
    else
    {
        printf("open base success\n");
    }
    ret = uart_init( fd_chassis, 115200L);
    if (ret == ERR)
    {
        perror("uart init error\n");
        return ERR;
    }
    else
    {
        printf("car base init success\n");
    }

//    /**
//     * @brief :初始化蓝牙串口
//     */
//
//    char *uart_BLE_dev = UART_BLE_NAME;
//    int ret_BLE = ERR;
//
//    fd_BLE = open(uart_BLE_dev, O_RDWR);
//    if (fd_BLE == ERR) {
//        perror("open BLE_file fail\n");
//        return ERR;
//    }
//    ret_BLE = uart_init(fd_BLE, 9600L);
//    if (ret_BLE == ERR) {
//        perror("uart_BLE init error\n");
//        return ERR;
//    }
//
//    /**
//     *  @brief IMU 串口初始化
//     */
    char *uart_imu_dev = UART_TTL_IMU_NAME ;
    if((fd_imu = serial_open(uart_imu_dev , 115200) < 0))
    {
        printf("open fail\n");
        return 0;
    }
    else printf("open success\n");
    int imu_ret = uart_init(fd_imu, 115200L);
    if (ret == ERR)
    {
        perror("uart init error\n");
        return ERR;
    }

    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    WitRegisterCallBack(SensorDataUpdata);

    /**
     * @brief  SDK 自动扫描功能
     */
     /**
      * 应该是少了这个自动扫描的这部分
      */
    /**
     * @brief  SDK 自动扫描功能
     */


    /**
     * @brief 线程启动
     */
    pthread_t pid_t1;
    //pthread_t pid_t2;
    //pthread_t pid_t3;
    //pthread_t pid_t4;
    pthread_create(&pid_t1, NULL, (void *)_serial_output_task, 0);
    //pthread_create(&pid_t2, NULL, (void *)_serial_BLE_input_task, 0);
    // pthread_create(&pid_t3,NULL,(void *)_serial_IMU_input_task,0);
    //pthread_create(&pid_t4,NULL,(void *)_serial_LORA_OUTPUT_task,0);
    //pthread_create(&pid_t4,NULL,(void *)_serial_LORA_OUTPUT_task,0);

    while (1)
    {
        
        sleep(10L);
    }
    close(fd_chassis);
    return 0;
}
