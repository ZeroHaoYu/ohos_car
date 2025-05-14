#ifndef __UM_GPIO_H__
#define __UM_GPIO_H__

#define UM_GPIO_EXPORT "/sys/class/gpio/export"
#define UM_GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define UM_GPIO_PEX "/sys/class/gpio/gpio"

// hilog
#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0  // 标识业务领域，范围0x0~0xFFFFF
#define LOG_TAG "GPIO_TEST"

// gpios
#define GPIO_23 36 /* GPIO1_A4 */
#define GPIO_24 106 /* GPIO3_B2 */
#define GPIO_25 125 /* GPIO3_D5 */
#define GPIO_16 137 /* GPIO4_B1 */
#define GPIO_05 135 /* GPIO4_A7 */
#define GPIO_06 134 /* GPIO4_A6 */

// direction
#define UM_GPIO_DIRECTION_IN 0
#define UM_GPIO_DIRECTION_OUT 1

// is export
#define UM_GPIO_NOT_EXPORT 0
#define UM_GPIO_EXPORTED 1

// errno
#define UM_GPIO_ERR (-1)
#define UM_GPIO_NOT_EXPROT_ERROR (-2)

// value high - low level
#define UM_GPIO_LOW_LEVE 0
#define UM_GPIO_HIGH_LEVE 1

/**
 * set gpio export
 * @param gpioNum gpioNum
 * @param bExport export,0:not export 1:export
 */
int UM_GPIO_Export(int gpioNum, int bExport);

/**
 * set gpio direction
 * @param gpioNum gpioNum
 * @param direction direction,0:in 1:out
 */
int UM_GPIO_SetDirection(int gpioNum, int direction);

/**
 * set gpio value
 * @param gpioNum gpioNum
 * @param value value,0:low 1:high
 */
int UM_GPIO_SetValue(int gpioNum, int value);

/**
 * check gpio export or not
 * @param gpioNum gpioNum
 * @param *value export,0:not export 1:exported
 */
int UM_GPIO_IsExport(int gpioNum, int *value);

/**
 * get gpio direction
 * @param gpioNum gpioNum
 * @param *value direction,0:in 1:out
 */
int UM_GPIO_GetDirection(int gpioNum, int *value);

/**
 * get gpio value
 * @param gpioNum gpioNum
 * @param *value value,0:low 1:high
 */
int UM_GPIO_GetValue(int gpioNum, int *value);

#endif /* __UM_GPIO_H__ */