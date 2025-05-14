#ifndef _RKNN_YOLOV5_DEMO_YOLO_H_
#define _RKNN_YOLOV5_DEMO_YOLO_H_

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define _BASETSD_H



#include "postprocess.h"

#include "rknn_api.h"
#include "preprocess.h"

#define PERF_WITH_POST 1

static void dump_tensor_attr(rknn_tensor_attr *attr);
double __get_us(struct timeval t);
static unsigned char *load_data(FILE *fp, size_t ofst, size_t sz);
static unsigned char *load_model(const char *filename, int *model_size);
static int saveFloat(const char *file_name, float *output, int element_size);
int process(cv::Mat &inputData);



#endif //_RKNN_YOLOV5_DEMO_PREPROCESS_H_