#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "securec.h"
#include "hilog/log.h"
#include "um_pwm.h"

int init_pwm(int pwmChannel)
{
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/export", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/export", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM EXPORT FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    fp = fopen(pwm_file_name, "w");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open export file!");
        return PWM_FILE_NOT_EXIST;
    }
    (void)fprintf(fp, "%d", 0);
    (void)fclose(fp);
    fp = NULL;
    return 0;
}

int set_pwm_period(int pwmChannel, int period)
{
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/period", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/period", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM PERIOD FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    fp = fopen(pwm_file_name, "r+");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open period file!");
        return PWM_FILE_NOT_EXIST;
    }
    if (period) {
        fprintf(fp, "%d", period);
    }
    (void)fclose(fp);
    fp = NULL;

    return 0;
}

int set_pwm_dutyCycle(int pwmChannel, int dutyCycle)
{
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/duty_cycle", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/duty_cycle", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM DUTY_CYCLE FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    fp = fopen(pwm_file_name, "r+");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open duty_cycle file!");
        return PWM_FILE_NOT_EXIST;
    }
    if (dutyCycle) {
        fprintf(fp, "%d", dutyCycle);
    }
    (void)fclose(fp);
    fp = NULL;

    return 0;
}

int set_pwm_polarity(int pwmChannel, int polarity)
{
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/polarity", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/polarity", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM POKARITY FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    fp = fopen(pwm_file_name, "rw+");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open polarity file!");
        return PWM_FILE_NOT_EXIST;
    }
    if (polarity == PWM_POLARITY_NORMAL) {
        fprintf(fp, "%s", "normal");
    } else if (polarity == PWM_POLARITY_INVERSED) {
        fprintf(fp, "%s", "inversed");
    }
    (void)fclose(fp);
    fp = NULL;

    return 0;
}

int set_pwm_enable(int pwmChannel, int isEnable)
{
    char buffer[256] = {0};
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/enable", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/enable", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM ENABLE FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    (void)snprintf_s(buffer, sizeof(buffer), sizeof(buffer), "echo %d > %s", isEnable, pwm_file_name);
    system(buffer);

    return 0;
}

/**
 * 根据PWM通道获取PWM周期
 * 
 * @param pwmChannel PWM通道编号
 * @return PWM周期值，如果通道错误或文件不存在则返回相应的错误码
 *
 * 本函数根据传入的PWM通道编号，构造对应的PWM周期文件路径，
 * 检查文件是否存在，并读取文件内容以获取PWM周期值。
 * 如果通道编号错误或周期文件不存在，则返回相应的错误码。
 */
int get_pwm_period(int pwmChannel)
{
    int ret = 0;
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/period", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/period", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM PERIOD FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    char buffer[32] = {0};
    (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
    fp = fopen(pwm_file_name, "r");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open period file!");
        return PWM_FILE_NOT_EXIST;
    }
    (void)fread(buffer, sizeof(buffer), 1, fp);
    (void)fclose(fp);
    fp = NULL;

    ret = atoi(buffer);
    return ret;
}
/**
 * @brief 根据PWM通道获取PWM占空比
 * 
 * 本函数通过指定的PWM通道（pwmChannel）来获取对应的PWM占空比。
 * 它首先确定请求的PWM通道是PWM1还是PWM0，然后构造相应的系统文件路径，
 * 接着检查该文件是否存在。如果文件存在，则打开文件，读取其中的数值（占空比），
 * 最后关闭文件并返回读取到的占空比值。如果通道非法或文件不存在，则返回错误码。
 * 
 * @param pwmChannel PWM通道标识符，用于区分不同的PWM通道。
 * @return int 返回PWM占空比或在错误情况下返回特定的错误码。
 */
int get_pwm_dutyCycle(int pwmChannel)
{
    int ret = 0;
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));
    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/duty_cycle", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/duty_cycle", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM DUTY_CYCLE FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    char buffer[32] = {0};
    (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
    fp = fopen(pwm_file_name, "r");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open duty_cycle file!");
        return PWM_FILE_NOT_EXIST;
    }
    (void)fread(buffer, sizeof(buffer), 1, fp);
    (void)fclose(fp);
    fp = NULL;

    ret = atoi(buffer);
    return ret;
}

int get_pwm_polarity(int pwmChannel)
{
    int ret = 0;
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/polarity", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/polarity", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM POLARITY FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    char buffer[32] = {0};
    (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
    fp = fopen(pwm_file_name, "r");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open polarity file!");
        return PWM_FILE_NOT_EXIST;
    }
    (void)fread(buffer, sizeof(buffer), 1, fp);
    (void)fclose(fp);
    fp = NULL;

    if (strstr(buffer, "normal") != NULL) {
        ret = PWM_POLARITY_NORMAL;
    } else if (strstr(buffer, "inversed") != NULL) {
        ret = PWM_POLARITY_INVERSED;
    } else {
        HILOG_ERROR(LOG_CORE, "PWM ERROR \n");
        ret = PWM_ERR;
    }

    return ret;
}

int is_pwm_enabled(int pwmChannel)
{
    int ret = 0;
    char pwm_file_name[128] = {0};
    (void)memset_s(pwm_file_name, sizeof(pwm_file_name), 0, sizeof(pwm_file_name));

    if (PWM1 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/enable", PWM1_PEX);
    } else if (PWM0 == pwmChannel) {
        (void)sprintf_s(pwm_file_name, sizeof(pwm_file_name), "%s/pwm0/enable", PWM0_PEX);
    } else {
        HILOG_ERROR(LOG_CORE, "PWM WRONOG CHANEEL\n");
        return PWM_WRONOG_CHANNEL;
    }

    if (access(pwm_file_name, F_OK) != 0) {
        HILOG_ERROR(LOG_CORE, "PWM ENABLE FILE NOT EXIST\n");
        return PWM_FILE_NOT_EXIST;
    }

    FILE *fp = NULL;
    char buffer[32] = {0};
    (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
    fp = fopen(pwm_file_name, "r");
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "Failed to open enable file!");
        return PWM_FILE_NOT_EXIST;
    }
    (void)fread(buffer, sizeof(buffer), 1, fp);
    (void)fclose(fp);
    fp = NULL;

    ret = atoi(buffer);
    return ret;
}
