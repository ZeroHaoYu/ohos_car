#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "gpio.h"

int main(int argc, char **argv)
{
    int gpioNum = GPIO_23;
    int bExport = UM_GPIO_EXPORTED;
    int direction = UM_GPIO_DIRECTION_OUT;
    int value = UM_GPIO_HIGH_LEVE;
    int getValue = -1;

    // 判断是否有输入参数，如有，则赋值指定gpio口
    printf("argv[1] = %s, argv[2] = %s\n", argv[1], argv[2]);
    if (argv[1] != NULL) {
        getValue = atoi(argv[1]);
        if (getValue == GPIO_23 ) {
            gpioNum = getValue;
        } else {
            printf("please input the gpioNum 36.\n");
            return UM_GPIO_ERR;
        }
    }

    if (argv[2] != NULL) {
        getValue = atoi(argv[2]);
        if (getValue == 0)
            value = UM_GPIO_LOW_LEVE;
        else
            value = UM_GPIO_HIGH_LEVE;
    }

    // 判断gpio口是否已经导出，如未导出则执行对应函数
    UM_GPIO_IsExport(gpioNum, &getValue);
    if (getValue == UM_GPIO_NOT_EXPORT) {
        UM_GPIO_Export(gpioNum, bExport);
    }
    
    // 设置gpio口为输入或输出模式
    UM_GPIO_SetDirection(gpioNum, direction);
    while(1){
        if(value == UM_GPIO_LOW_LEVE)
            value = UM_GPIO_HIGH_LEVE;
        else
            value = UM_GPIO_LOW_LEVE;

        UM_GPIO_SetValue(gpioNum, value);

        // 获取对应gpio口的模式并打印
        UM_GPIO_GetDirection(gpioNum, &getValue);
        printf("gpioNum:[%d], direction:[%d]\n", gpioNum, getValue);

        // 获取对应gpio口的电平值并打印
        UM_GPIO_GetValue(gpioNum, &getValue);
        printf("gpioNum:[%d], value:[%d]\n", gpioNum, getValue);
        usleep(500000);
    }
    return 0;
}