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
#include <unistd.h>
#include <stdio.h>

#include "securec.h"
#include "hilog/log.h"

#include "i2cinterface.h"

#define DEVICE_ADDR 0x45

void user_help(void)
{
    printf("Please input like this: i2c_test <int> <int> \n");
    printf("\t\tthe first is to choice the mps range[0,4] \n");
    printf("\t\tthe second is to choice the repeatability range[0,2] \n");
}

int main(int argc, char *argv[])
{
    char *dev_name = "/dev/i2c-5";
    int result = -1;
    uint8_t mps = 0;
    uint8_t repeatability = 0;

    sStatusReg_t regRaw;
    sRHAndTemp_t tempRH = {0};

    if (argc < 2L || argc > 3L) {
        user_help();
        return 0;
    }

    mps = atoi(argv[1]);
    repeatability = atoi(argv[2L]);
    if (mps < 0 || mps > 4L || repeatability < 0 || repeatability > 2L) {
        user_help();
        return 0;
    }

    SoftReset(dev_name, DEVICE_ADDR);
    usleep(50L * 1000L); // 等待重启完成

    // 这里仅为测试读取状态寄存器是否正常，具体应用需用户进行编写
    result = ReadStatusReg(dev_name, DEVICE_ADDR, &regRaw);
    if (result != 0) {
        HILOG_ERROR(LOG_CORE, "ReadStatusRegister fail! \n");
        return result;
    }
    usleep(50L * 1000L); // 等待读取寄存器完成

    ModeSet(dev_name, DEVICE_ADDR, mps, repeatability);
    usleep(50L * 1000L); // 等待更改模式完成

    result = ReadTempAndHum(dev_name, DEVICE_ADDR, &tempRH);
    if (result != 0) {
        HILOG_ERROR(LOG_CORE, "ReadTemperatureAndHumidity fail! \n");
        return result;
    }

    printf(" TemperatureC: %f °C \n Humidity: %f %%RH \n TemperatureF: %f °F \n", tempRH.TemperatureC, tempRH.Humidity,
           tempRH.TemperatureF);

    return 0;
}
