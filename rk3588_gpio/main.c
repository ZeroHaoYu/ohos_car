#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "gpio.h"

int getPhysicalPin(const char *gpioNum) {
    if (!strcmp(gpioNum, "quit") || !strcmp(gpioNum, "exit")) {
        return -1;
    }
    else if (strcmp(gpioNum, "gpio23") == 0) {
        return GPIO_23;
    } 
    else if (strcmp(gpioNum, "gpio24") == 0) {
        return GPIO_24;
    }
     else if (strcmp(gpioNum, "gpio25") == 0) {
        return GPIO_25;
    } 
    else if (strcmp(gpioNum, "gpio16") == 0) {
        return GPIO_16;
    } 
    else if (strcmp(gpioNum, "gpio05") == 0) {
        return GPIO_05;
    } 
    else if (strcmp(gpioNum, "gpio06") == 0) {
        return GPIO_06;
    }
    else {
        return 0; // 无效的 GPIO 编号
    }
}
int main(void)
{
    char gpioNum[20];  //对应的开发板上的物理引脚号
    int dummy_pin = -1;      // 物理引脚号映射到系统文件上的虚拟引脚号
    int bExport = UM_GPIO_EXPORTED;
    int direction = UM_GPIO_DIRECTION_OUT;
    int value = UM_GPIO_HIGH_LEVE;   //你输入的数值
    int time = 10;
    int getValue = -1;                //函数捕捉到的引脚电平
    printf("Please enter the gpioNum number and value:\n");
    while (1)
    {
        scanf("%s %d", gpioNum, &value);
        
        if ((dummy_pin = getPhysicalPin(gpioNum)) < 0){
            printf("Invalid dummy_pin number\n");
            exit(-1);
        }
        // 判断gpio口是否已经导出，如未导出则执行对应函数
        UM_GPIO_IsExport(dummy_pin, &getValue);
        if (getValue == UM_GPIO_NOT_EXPORT) {
            UM_GPIO_Export(dummy_pin, bExport);
        }
        // 设置gpio口为输入或输出模式
        UM_GPIO_SetDirection(dummy_pin, direction);

        if (value != 2)
        {
            UM_GPIO_SetValue(dummy_pin, value);
            printf("gpioNum:[%s], value:[%d]\n", gpioNum, value);
        }
        else{
            while(time--){
                if (value == UM_GPIO_LOW_LEVE)
                    value = UM_GPIO_HIGH_LEVE;
                else
                    value = UM_GPIO_LOW_LEVE;

                UM_GPIO_SetValue(dummy_pin, value);
                printf("gpioNum:[%s], value:[%d]\n", gpioNum, value);
                usleep(200000);
            }
        }
        memset(gpioNum, 0, sizeof(gpioNum));
    }
    return 0;
}