/*
 * Copyright (c) 2022 Unionman Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "securec.h"
#include "hilog/log.h"

#include "i2cinterface.h"

/* 函数返回值说明 return 0 代表正常运行     return -1 代表驱动无法打开
                 return -2 代表分配缓存失败 return -3 代表读取/写入失败
                 return -4 代表CRC校验失败  return -5 代表保存数据失败  */

// 转换温湿度并保存在结构体中
void ConvertTempAndHum(sRHAndTemp_t *tempRH, uint8_t *rawTemp, uint8_t *rawHum)
{
    tempRH->TemperatureC = ConvertTemperature(rawTemp);
    tempRH->Humidity = ConvertHumidity(rawHum);
    tempRH->TemperatureF = 1.8f * tempRH->TemperatureC + 32L;
}

// 温度转换函数
float ConvertTemperature(uint8_t *rawTemp)
{
    uint16_t rawValue = 0;
    rawValue = rawTemp[0];
    rawValue = (rawValue << 8L) | rawTemp[1];
    return 175.0f * (float)rawValue / 65535.0f - 45.0f;
}

// 湿度转换函数
float ConvertHumidity(uint8_t *rawHum)
{
    uint16_t rawValue = 0;
    rawValue = rawHum[0];
    rawValue = (rawValue << 8L) | rawHum[1];
    return 100.0f * (float)rawValue / 65535.0f;
}

// CRC校验
uint8_t CheckCrc(uint8_t *data)
{
    uint8_t bit = 0;
    uint8_t crc = 0xFF;

    for (uint8_t dataCounter = 0; dataCounter < 2L; dataCounter++) {
        crc ^= (data[dataCounter]);
        for (bit = 8L; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

int SoftReset(char *devName, char addr)
{
    int fd = -1;
    int ret = -1;
    uint8_t _pBuf[2L] = {0};

    struct i2c_rdwr_ioctl_data i2c_data;

    // 关闭周期测量
    SendCommand(devName, addr, SHT3X_CMD_STOP_PERIODIC_ACQUISITION_MODE);
    usleep(50L * 1000L); // 等待发送控制命令完成

    fd = open(devName, O_RDWR);
    if (fd < 0) {
        HILOG_ERROR(LOG_CORE, "Can not have dev %s \n", devName);
        return -1;
    }

    i2c_data.nmsgs = 1;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL) {
        HILOG_ERROR(LOG_CORE, "malloc error \n");
        close(fd);
        return -2L;
    }

    ioctl(fd, I2C_TIMEOUT, 1);
    ioctl(fd, I2C_RETRIES, 2L);

    _pBuf[0] = SHT3X_CMD_SOFT_RESET >> 8L;
    _pBuf[1] = SHT3X_CMD_SOFT_RESET & 0xFF;

    // 写寄存器
    i2c_data.msgs[0].len = 2L;
    i2c_data.msgs[0].addr = addr;
    i2c_data.msgs[0].flags = 0; // 写:0 读:1
    i2c_data.msgs[0].buf = _pBuf;

    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0) {
        HILOG_ERROR(LOG_CORE, "SoftReset %c error \n", addr);
        close(fd);
        free(i2c_data.msgs);
        return -3L;
    }
    free(i2c_data.msgs);
    close(fd);

    return 0;
}

int SendCommand(char *devName, char addr, uint16_t modeCommand)
{
    int fd = -1;
    int ret = -1;
    uint8_t _pBuf[2L] = {0};

    struct i2c_rdwr_ioctl_data i2c_data;

    fd = open(devName, O_RDWR);
    if (fd < 0) {
        HILOG_ERROR(LOG_CORE, "can not have dev %s \n", devName);
        return -1;
    }

    i2c_data.nmsgs = 1;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL) {
        HILOG_ERROR(LOG_CORE, "malloc error \n");
        close(fd);
        return -2L;
    }

    ioctl(fd, I2C_TIMEOUT, 1);
    ioctl(fd, I2C_RETRIES, 2L);

    _pBuf[0] = modeCommand >> 8L;
    _pBuf[1] = modeCommand & 0xFF;
    // 写寄存器
    i2c_data.msgs[0].len = 2L;
    i2c_data.msgs[0].addr = addr;
    i2c_data.msgs[0].flags = 0; // 写:0 读:1
    i2c_data.msgs[0].buf = _pBuf;

    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0) {
        HILOG_ERROR(LOG_CORE, "SendCommand %c error \n", addr);
        close(fd);
        free(i2c_data.msgs);
        return -3L;
    }
    free(i2c_data.msgs);
    close(fd);

    return 0;
}

// 读状态寄存器
int ReadStatusReg(char *devName, char addr, sStatusReg_t *regRaw)
{
    uint8_t register1[3L] = {0};
    uint16_t data = 0;

    int fd = -1;
    int ret = -1;

    uint8_t _pBuf[2L] = {0};

    struct i2c_rdwr_ioctl_data i2c_data;

    fd = open(devName, O_RDWR);
    if (fd < 0) {
        HILOG_ERROR(LOG_CORE, "can not have dev %s \n", devName);
        return -1;
    }

    i2c_data.nmsgs = 2L;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL) {
        HILOG_ERROR(LOG_CORE, "malloc error \n");
        close(fd);
        return -2L;
    }

    ioctl(fd, I2C_TIMEOUT, 1);
    ioctl(fd, I2C_RETRIES, 2L);

    _pBuf[0] = SHT3X_CMD_READ_STATUS_REG >> 8L;
    _pBuf[1] = SHT3X_CMD_READ_STATUS_REG & 0xFF;
    // 写寄存器
    i2c_data.msgs[0].len = 2L;
    i2c_data.msgs[0].addr = addr;
    i2c_data.msgs[0].flags = 0; // 写:0 读:1
    i2c_data.msgs[0].buf = _pBuf;
    // 读取数据
    i2c_data.msgs[1].len = 3L;
    i2c_data.msgs[1].addr = addr;
    i2c_data.msgs[1].flags = 1; // 写:0 读:1
    i2c_data.msgs[1].buf = register1;

    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0) {
        HILOG_ERROR(LOG_CORE, "ReadStatusRegister read data %c  error \n", addr);
        close(fd);
        free(i2c_data.msgs);
        return -3L;
    }
    free(i2c_data.msgs);
    close(fd);

    if (CheckCrc(register1) != register1[2L]) {
        HILOG_ERROR(LOG_CORE, "ReadStatusRegister data checkCrc fail \n");
        return -4L;
    }

    data = (register1[0] << 8L) | register1[1];
    ret = memcpy_s(regRaw, sizeof(sStatusReg_t), &data, 2L);
    if (ret != 0) {
        HILOG_ERROR(LOG_CORE, "ReadStatusRegister memcpy_s fail! \n");
        return -5L;
    }

    return 0;
}

// 读温湿度
int ReadTempAndHum(char *devName, char addr, sRHAndTemp_t *tempRH)
{
    int fd = -1;
    int ret = -1;

    uint8_t rawData[6L] = {0};
    uint8_t rawTemp[3L] = {0};
    uint8_t rawHum[3L] = {0};

    struct i2c_rdwr_ioctl_data i2c_data;

    fd = open(devName, O_RDWR);
    if (fd < 0) {
        HILOG_ERROR(LOG_CORE, "can not have dev %s \n", devName);
        return -1;
    }

    i2c_data.nmsgs = 1;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL) {
        HILOG_ERROR(LOG_CORE, "malloc error \n");
        close(fd);
        return -2L;
    }

    ioctl(fd, I2C_TIMEOUT, 1);
    ioctl(fd, I2C_RETRIES, 2L);

    // 读取数据
    i2c_data.msgs[0].len = 6L;
    i2c_data.msgs[0].addr = addr;
    i2c_data.msgs[0].flags = 1; // 写:0 读:1
    i2c_data.msgs[0].buf = rawData;

    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0) {
        HILOG_ERROR(LOG_CORE, "ReadTempAndHum read data 0x%c  error \n", addr);
        close(fd);
        free(i2c_data.msgs);
        return -3L;
    }
    free(i2c_data.msgs);
    close(fd);

    ret = memcpy_s(rawTemp, sizeof(rawTemp), rawData, 3L);
    if (ret != 0) {
        HILOG_ERROR(LOG_CORE, "ReadTempAndHum memcpy_s fail! \n");
        return -5L;
    }

    ret = memcpy_s(rawHum, sizeof(rawHum), rawData + 3L, 3L);
    if (ret != 0) {
        HILOG_ERROR(LOG_CORE, "ReadTempAndHum memcpy_s fail! \n");
        return -5L;
    }

    if ((CheckCrc(rawTemp) != rawTemp[2L]) || (CheckCrc(rawHum) != rawHum[2L])) {
        HILOG_ERROR(LOG_CORE, "ReadTempAndHum data checkCrc fail \n");
        return -4L;
    }

    ConvertTempAndHum(tempRH, rawTemp, rawHum);

    return 0;
}

// 设置mps与刷新率
int ModeSet(char *devName, char addr, uint8_t mps, uint8_t repeatability)
{
    int ret;
    if (mps == 0) {
        ret = MpsHalfHz(devName, addr, repeatability);
    } else if (mps == 1) {
        ret = Mps1Hz(devName, addr, repeatability);
    } else if (mps == 2L) {
        ret = Mps2Hz(devName, addr, repeatability);
    } else if (mps == 3L) {
        ret = Mps4Hz(devName, addr, repeatability);
    } else if (mps == 4L) {
        ret = Mps10Hz(devName, addr, repeatability);
    } else {
        HILOG_ERROR(LOG_CORE, " mps int for range[0,4]\n repeatability int for range[0,2] \n");
        ret = -3L;
    }
    return ret;
}

// mps为0.5hz时候选择刷新率
int MpsHalfHz(char *devName, char addr, uint8_t repeatability)
{
    int ret;
    if (repeatability == 0) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_L_FREQUENCY_HALF_HZ);
        ret = 0;
    } else if (repeatability == 1) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_M_FREQUENCY_HALF_HZ);
        ret = 0;
    } else if (repeatability == 2L) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_H_FREQUENCY_HALF_HZ);
        ret = 0;
    } else {
        HILOG_ERROR(LOG_CORE, " mps int for range[0,4]\n repeatability int for range[0,2] \n");
        ret = -3L;
    }
    return ret;
}

// mps为1hz时候选择刷新率
int Mps1Hz(char *devName, char addr, uint8_t repeatability)
{
    int ret;
    if (repeatability == 0) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_L_FREQUENCY_1_HZ);
        ret = 0;
    } else if (repeatability == 1) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_M_FREQUENCY_1_HZ);
        ret = 0;
    } else if (repeatability == 2L) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_H_FREQUENCY_1_HZ);
        ret = 0;
    } else {
        HILOG_ERROR(LOG_CORE, " mps int for range[0,4]\n repeatability int for range[0,2] \n");
        ret = -3L;
    }
    return ret;
}

// mps为2hz时候选择刷新率
int Mps2Hz(char *devName, char addr, uint8_t repeatability)
{
    int ret;
    if (repeatability == 0) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_L_FREQUENCY_2_HZ);
        ret = 0;
    } else if (repeatability == 1) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_M_FREQUENCY_2_HZ);
        ret = 0;
    } else if (repeatability == 2L) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_H_FREQUENCY_2_HZ);
        ret = 0;
    } else {
        HILOG_ERROR(LOG_CORE, " mps int for range[0,4]\n repeatability int for range[0,2] \n");
        ret = -3L;
    }
    return ret;
}

// mps为4hz时候选择刷新率
int Mps4Hz(char *devName, char addr, uint8_t repeatability)
{
    int ret;
    if (repeatability == 0) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_L_FREQUENCY_4_HZ);
        ret = 0;
    } else if (repeatability == 1) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_M_FREQUENCY_4_HZ);
        ret = 0;
    } else if (repeatability == 2L) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_H_FREQUENCY_4_HZ);
        ret = 0;
    } else {
        HILOG_ERROR(LOG_CORE, " mps int for range[0,4]\n repeatability int for range[0,2] \n");
        ret = -3L;
    }
    return ret;
}

// mps为10hz时候选择刷新率
int Mps10Hz(char *devName, char addr, uint8_t repeatability)
{
    int ret;
    if (repeatability == 0) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_L_FREQUENCY_10_HZ);
        ret = 0;
    } else if (repeatability == 1) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_M_FREQUENCY_10_HZ);
        ret = 0;
    } else if (repeatability == 2L) {
        SendCommand(devName, addr, SHT3X_CMD_SETMODE_H_FREQUENCY_10_HZ);
        ret = 0;
    } else {
        HILOG_ERROR(LOG_CORE, " mps int for range[0,4]\n repeatability int for range[0,2] \n");
        ret = -3L;
    }
    return ret;
}