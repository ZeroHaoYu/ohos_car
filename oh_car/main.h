/**
  ******************************************************************************
  * @file           : main.h
  * @author         : Dylan
  * @brief          : 主函数公共基础库和私库头文件
  * @date           : 2023.3.22
  ******************************************************************************
  */

#ifndef OH_CAR_CHASSIS_CONTROL_MAIN_H
#define OH_CAR_CHASSIS_CONTROL_MAIN_H

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "pthread_usr.h"
#include "wit_c_sdk.h"
#include "serial.h"
#include "REG.h"
#include "serial_protocol.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif //OH_CAR_CHASSIS_CONTROL_MAIN_H
