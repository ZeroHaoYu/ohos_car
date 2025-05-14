/**
******************************************************************************
* @file           : Pthread_usr.h
* @author         : Dylan
* @brief          : 线程实现
* @attention      : 包含了串口文件符
* @date           : 2023.3.22
******************************************************************************
*/
#ifndef OH_CAR_CHASSIS_CONTROL_PTHREAD_USR_H
#define OH_CAR_CHASSIS_CONTROL_PTHREAD_USR_H

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
#include <math.h>


#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
* @brief           :自定义库
*/
#include "serial_base.h"
#include "serial_protocol.h"
#include "REG.h"
#include "serial.h"
#include "wit_c_sdk.h"
#include <stdint.h>
#include "controller.h"

#define PORT 8080
#define BUFFER_SIZE 1024


/**
* @type            :串口标识符
* @brief           :底盘串口表示符
*/
extern int fd_chassis;

/*
* @type             :IMU标识符
* @brief            :IMU串口表示符
*/
extern int fd_imu;



extern volatile char s_cDataUpdate;




/**
* @brief           :串口接受线程
* @return          :none
* @state           :未启用
*/
void *_serial_input_task(void);
/**
* @brief           :串口输出线程
* @return          :none
*/
void _serial_output_task(void);
/**
* @brief           :IMU串口打印线程
* @return          :none
*/
void _serial_IMU_input_task(void);

void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);

/**
* @brief           :tcp网络连接线程
* @return          :none
*/
void _tcp_server_socket(void);

/**
* @brief           :normalize func
* @return          :none
*/
float normalizeAngle(float angle);

/**
* @brief           :compute gravityY
* @return          :none
*/
float computeGravityY(float pitchAngle);


#endif //OH_CAR_CHASSIS_CONTROL_PTHREAD_USR_H

