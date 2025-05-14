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

#ifndef _I2CINTERFACE_H_
#define _I2CINTERFACE_H_

#ifdef _cplusplus
extern "C" {
#endif

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0 // 标识业务领域，范围0x0~0xFFFFF
#define LOG_TAG "i2c_test"

#define SHT3X_CMD_READ_SERIAL_NUMBER (0x3780)    // Read the chip serial number
#define SHT3X_CMD_GETDATA_H_CLOCKENBLED (0x2C06) // Measurement:high repeatability
#define SHT3X_CMD_GETDATA_M_CLOCKENBLED (0x2C0D) // Measurement: medium repeatability
#define SHT3X_CMD_GETDATA_L_CLOCKENBLED (0x2C10) // Measurement: low repeatability

#define SHT3X_CMD_SETMODE_H_FREQUENCY_HALF_HZ (0x2032) // Measurement: periodic 0.5 mps, high repeatability
#define SHT3X_CMD_SETMODE_M_FREQUENCY_HALF_HZ (0x2024) // Measurement: periodic 0.5 mps, medium
#define SHT3X_CMD_SETMODE_L_FREQUENCY_HALF_HZ (0x202F) // Measurement: periodic 0.5 mps, low repeatability
#define SHT3X_CMD_SETMODE_H_FREQUENCY_1_HZ (0x2130)    // Measurement: periodic 1 mps, high repeatability
#define SHT3X_CMD_SETMODE_M_FREQUENCY_1_HZ (0x2126)    // Measurement: periodic 1 mps, medium repeatability
#define SHT3X_CMD_SETMODE_L_FREQUENCY_1_HZ (0x212D)    // Measurement: periodic 1 mps, low repeatability
#define SHT3X_CMD_SETMODE_H_FREQUENCY_2_HZ (0x2236)    // Measurement: periodic 2 mps, high repeatability
#define SHT3X_CMD_SETMODE_M_FREQUENCY_2_HZ (0x2220)    // Measurement: periodic 2 mps, medium repeatability
#define SHT3X_CMD_SETMODE_L_FREQUENCY_2_HZ (0x222B)    // Measurement: periodic 2 mps, low repeatability
#define SHT3X_CMD_SETMODE_H_FREQUENCY_4_HZ (0x2334)    // Measurement: periodic 4 mps, high repeatability
#define SHT3X_CMD_SETMODE_M_FREQUENCY_4_HZ (0x2322)    // Measurement: periodic 4 mps, medium repeatability
#define SHT3X_CMD_SETMODE_L_FREQUENCY_4_HZ (0x2329)    // Measurement: periodic 4 mps, low repeatability
#define SHT3X_CMD_SETMODE_H_FREQUENCY_10_HZ (0x2737)   // Measurement: periodic 10 mps, high repeatability
#define SHT3X_CMD_SETMODE_M_FREQUENCY_10_HZ (0x2721)   // Measurement: periodic 10 mps, medium
#define SHT3X_CMD_SETMODE_L_FREQUENCY_10_HZ (0x272A)   // Measurement: periodic 10 mps, low repeatability
#define SHT3X_CMD_GETDATA (0xE000)                     // Readout measurements for periodic mode

#define SHT3X_CMD_STOP_PERIODIC_ACQUISITION_MODE (0x3093)
#define SHT3X_CMD_SOFT_RESET (0x30A2)       // Soft reset
#define SHT3X_CMD_HEATER_ENABLE (0x306D)    // Enabled heater
#define SHT3X_CMD_HEATER_DISABLE (0x3066)   // Disable heater
#define SHT3X_CMD_READ_STATUS_REG (0xF32D)  // Read status register
#define SHT3X_CMD_CLEAR_STATUS_REG (0x3041) // Clear status register

#define SHT3X_CMD_READ_HIGH_ALERT_LIMIT_SET (0xE11F)    // Read alert limits, high set
#define SHT3X_CMD_READ_HIGH_ALERT_LIMIT_CLEAR (0xE114)  // Read alert limits, high clear
#define SHT3X_CMD_READ_LOW_ALERT_LIMIT_CLEAR (0xE109)   // Read alert limits, low clear
#define SHT3X_CMD_READ_LOW_ALERT_LIMIT_SET (0xE102)     // Read alert limits, low set
#define SHT3X_CMD_WRITE_HIGH_ALERT_LIMIT_SET (0x611D)   // Write alert limits, high set
#define SHT3X_CMD_WRITE_HIGH_ALERT_LIMIT_CLEAR (0x6116) // Write alert limits, high clear
#define SHT3X_CMD_WRITE_LOW_ALERT_LIMIT_CLEAR (0x610B)  // Write alert limits, low clear
#define SHT3X_CMD_WRITE_LOW_ALERT_LIMIT_SET (0x6100)    // Write alert limits, low set

typedef struct {
    uint8_t writeDataChecksumStatus : 1;
    uint8_t commandStatus : 1;
    uint8_t reserved0 : 2;
    uint8_t systemResetDeteced : 1;
    uint8_t reserved1 : 5;
    uint8_t temperatureAlert : 1;
    uint8_t humidityAlert : 1;
    uint8_t reserved2 : 1;
    uint8_t heaterStaus : 1;
    uint8_t reserved3 : 1;
    uint8_t alertPendingStatus : 1;
} __attribute__((packed)) sStatusReg_t;

/**
Structures used to store temperature and relative humidity
*/
typedef struct {
    float TemperatureC;
    float Humidity;
    float TemperatureF;
} sRHAndTemp_t;

void ConvertTempAndHum(sRHAndTemp_t *tempRH, uint8_t *rawTemp, uint8_t *rawHum);
float ConvertTemperature(uint8_t *rawTemp);
float ConvertHumidity(uint8_t *rawHum);
uint8_t CheckCrc(uint8_t *data);
int SoftReset(char *devName, char addr);
int ReadStatusReg(char *devName, char addr, sStatusReg_t *regRaw);
int SendCommand(char *devName, char addr, uint16_t modeCommand);
int ModeSet(char *devName, char addr, uint8_t mps, uint8_t repeatability);
int ReadTempAndHum(char *devName, char addr, sRHAndTemp_t *tempRH);

int MpsHalfHz(char *devName, char addr, uint8_t repeatability);
int Mps1Hz(char *devName, char addr, uint8_t repeatability);
int Mps2Hz(char *devName, char addr, uint8_t repeatability);
int Mps4Hz(char *devName, char addr, uint8_t repeatability);
int Mps10Hz(char *devName, char addr, uint8_t repeatability);

#ifdef _cplusplus
}
#endif

#endif
