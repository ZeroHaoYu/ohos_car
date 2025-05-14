
#ifndef SERIAL_H
#define SERIAL_H

#include <linux/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/rtc.h>
#include <termios.h>

#include "pthread_usr.h"
#include "wit_c_sdk.h"
#include "serial.h"
#include "REG.h"
#include "serial_protocol.h"

#define IMU_TTL_NAME "/dev/ttyUSB1"

void serial_close(int fd);

int serial_open(char* dev, int baud);

int serial_read_data(int fd, char *val, int len);

int serial_write_data(int fd, char *val, int len);

void AutoScanSensor(char* dev);

#endif



extern const int c_uiBaud[];
extern volatile char s_cDataUpdate;