#include <linux/videodev2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <malloc.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include "camera.h"

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    int index = 0;
    int ret = 0;
    int i = 0;
    char buf[27] = {0};
    struct _v4l2_video video;
    fd_set fds;
    struct timeval tv;

    video.fd = open_video_device("/dev/video21");
    if (video.fd < 0)
    {
        return -1;
    }
    ret = query_video_device_cap(&video);
    if (ret < 0)
    {
        goto __exit;
    }

    ret = set_video_device_par(&video);
    if (ret < 0)
    {
        goto __exit;
    }

    ret = set_video_device_mmap(&video);
    if (ret < 0)
    {
        goto __exit;
    }

    ret = set_video_device_stream_on(&video);
    if (ret < 0)
    {
        goto __exit;
    }

    for (i = 0; i < 3; i++)
    { /* 采集3张(帧)图片 */
        FD_ZERO(&fds);
        FD_SET(video.fd, &fds);

        tv.tv_sec = 5; /* wait time */
        tv.tv_usec = 0;
        ret = select(video.fd + 1, &fds, NULL, NULL, &tv);

        if (ret < 0)
        {
            perror("select error");
            goto __exit;
        }
        else if (ret == 0)
        {
            printf("select timeout\n");
            goto __exit;
        }
        ret = read_video_device_stream_frame(&video, &index);
        if (ret < 0)
        {
            goto __exit;
        }
        sprintf(buf, "./picture/image%d.jpg", i);
        fp = fopen(buf, "wb"); /* 保存为图片文件 */
        if (fp == NULL)
        {
            perror("open image file failed\n");
            goto __exit;
        }
        printf("save %s \n", buf);
        fwrite(video.mmap_buf[index].addr, video.mmap_buf[index].size, 1, fp);
        fclose(fp);
        set_video_device_stream_queue(&video, index);
        usleep(1000);
    }
__exit:
    set_video_device_stream_off(&video);
    close_video_device(&video);

    return 0;
}
