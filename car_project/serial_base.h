/**
  ******************************************************************************
  * @file           : serial_base.h
  * @author         : Dylan
  * @brief          : 串口底层协议实现
  * @attention      : 除串口号定义,帧长度外 请勿改动
  * @date           :  2023.3.23 2023.3.22
  ******************************************************************************
  */
#ifndef OH_CAR_CHASSIS_CONTROL_SERIAL_BASE_H
#define OH_CAR_CHASSIS_CONTROL_SERIAL_BASE_H
/**
 * @brief           :串口寄存器和数据宏定义
 */
#define UART_TTL_NAME "/dev/ttyUSB2"          //底盘串口
#define UART_BLE_NAME "/dev/ttyUSB1"          // 蓝牙串口
#define UART_TTL_IMU_NAME "/dev/ttyUSB0"       //IMU 串口
#define UART_TTL_LORA_NAME "/dev/ttyUSB1"      //雷达 串口
#define FRAME_LEN 4         //底盘串口通讯帧长度
#define FRAME_LEN_BLE 1     //蓝牙串口通讯帧长度
#define OK 0
#define ERR (-1)

#define B0 0000000 /* hang up */
#define B50 0000001
#define B75 0000002
#define B110 0000003
#define B134 0000004
#define B150 0000005
#define B200 0000006
#define B300 0000007
#define B600 0000010
#define B1200 0000011
#define B1800 0000012
#define B2400 0000013
#define B4800 0000014
#define B9600 0000015
#define B19200 0000016
#define B38400 0000017
#ifdef __USE_MISC
#define EXTA B19200
#define EXTB B38400
#endif
#define CSIZE 0000060 /*数据位屏蔽*/
#define CS5 0000000   /*5,6,7,8为数据位*/
#define CS6 0000020
#define CS7 0000040
#define CS8 0000060
#define CSTOPB 0000100 /*停止位*/
#define CREAD 0000200  /*接收标志位*/
#define PARENB 0000400 /*奇偶校验位开启标志位*/
#define PARODD 0001000 /*奇校验，否则偶校验*/
#define HUPCL 0002000
#define CLOCAL 0004000 /*本地连接标志位*/

//#define CBAUDEX 0010000

#define B57600 0010001
#define B115200 0010002
#define B230400 0010003
#define B460800 0010004
#define B500000 0010005
#define B576000 0010006
#define B921600 0010007
#define B1000000 0010010
#define B1152000 0010011
#define B1500000 0010012
#define B2000000 0010013
#define B2500000 0010014
#define B3000000 0010015
#define B3500000 0010016
#define B4000000 0010017
#define __MAX_BAUD B4000000

#define CIBAUD 002003600000  /* input baud rate (not used) */
#define CMSPAR 010000000000  /* mark or space (stick) parity */
#define CRTSCTS 020000000000 /* flow control */


/**
 * @brief           :公共基础库
 */
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>


/**
 * @brief           :串口初始化  设定文件描述符和基础寄存器参数
 * @param fd_chassis
 * @param uartBaud
 * @return
 */
int uart_init(int fd_chassis, int uartBaud);





/**
 * @brief           :波特率转换函数
 * @param baudrate
 * @return
 */
speed_t conver_baudrate(int baudrate);



/**
 * @brief           :波特率设置函数
 * @param fd_chassis
 * @param baud
 */
void set_baud(int fd_chassis, int baud);


/**
 * @brief           ：数据位设置函数
 * @param setup_databits
 * @param options_databits
 * @return
 */
int setup_data_bits(int setup_databits, struct termios *options_databits);


/**
 * @brief           :校验位设置函数
 * @param setup_parity
 * @param options_parity
 * @return
 */
int set_params_parity(int setup_parity, struct termios *options_parity);


/**
 * @brief           ：停止位设置函数
 * @param fd_chassis
 * @param databits
 * @param stopbits
 * @param parity
 * @return
 */
int set_params(int fd_chassis, int databits, int stopbits, int parity);


#endif //OH_CAR_CHASSIS_CONTROL_SERIAL_BASE_H
