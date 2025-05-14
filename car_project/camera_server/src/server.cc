#include "server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;
/**
 * 打印张量属性信息
 * 
 * @param attr 张量属性结构体指针，包含指数、名称、维度等信息
 */
static void dump_tensor_attr(rknn_tensor_attr *attr) {
  // 初始化shape字符串，用于存储张量的维度信息
  std::string shape_str = attr->n_dims < 1 ? "" : std::to_string(attr->dims[0]);
  // 遍历张量的每个维度，将其转换为字符串并添加到shape_str中
  for (int i = 1; i < attr->n_dims; ++i) {
    shape_str += ", " + std::to_string(attr->dims[i]);
  }
  // 打印张量的详细属性信息，包括指数、名称、维度、元素数量、大小、步长等
  printf(
      "  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, "
      "w_stride = %d, size_with_stride=%d, fmt=%s, "
      "type=%s, qnt_type=%s, "
      "zp=%d, scale=%f\n",
      attr->index, attr->name, attr->n_dims, shape_str.c_str(), attr->n_elems,
      attr->size, attr->w_stride, attr->size_with_stride,
      get_format_string(attr->fmt), get_type_string(attr->type),
      get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}
double __get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }
static unsigned char *load_data(FILE *fp, size_t ofst, size_t sz) {
    unsigned char *data;
    int ret;
    data = NULL;
    if (NULL == fp) {
      return NULL;
    }
    ret = fseek(fp, ofst, SEEK_SET);
    if (ret != 0) {
      printf("blob seek failure.\n");
      return NULL;
    }
    data = (unsigned char *)malloc(sz);
    if (data == NULL) {
      printf("buffer malloc failure.\n");
      return NULL;
    }
    ret = fread(data, 1, sz, fp);
    return data;
}
static unsigned char *load_model(const char *filename, int *model_size) {
    FILE *fp;
    unsigned char *data;
    fp = fopen(filename, "rb");
    
    if (NULL == fp) {
        printf("Open file %s failed.\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    data = load_data(fp, 0, size);
    fclose(fp);
    *model_size = size;
    return data;
}
static int saveFloat(const char *file_name, float *output, int element_size) {
  FILE *fp;
  fp = fopen(file_name, "w");
  for (int i = 0; i < element_size; i++) {
    fprintf(fp, "%.6f\n", output[i]);
  }
  fclose(fp);
  return 0;
}
/*-------------------------------------------
                  Main Functions
-------------------------------------------*/
int ret;
rknn_context ctx;
size_t actual_size = 0;
int img_width = 0;
int img_height = 0;
int img_channel = 0;
const float nms_threshold = NMS_THRESH;
// 默认的NMS阈值
const float box_conf_threshold = BOX_THRESH;
// 默认的置信度阈值
struct timeval start_time, stop_time;
char *model_name = "/data/model/RK3588/yolov5s-640-640.rknn";
std::string option = "letterbox";
std::string out_path = "./out.jpg";
// init rga context
rga_buffer_t src;
rga_buffer_t dst;
int model_data_size = 0;
unsigned char *model_data;
rknn_sdk_version version;
rknn_input_output_num io_num;
rknn_tensor_attr input_attrs[1];
rknn_tensor_attr output_attrs[1];
int channel = 3;
int width = 0;
int height = 0;
rknn_input inputs[1];

/**
 * @brief 初始化函数
 * 
 * 该函数负责初始化神经网络模型，包括加载模型数据、初始化RKNN上下文、查询模型输入输出信息等
 * 
 * @return int 返回初始化结果，失败返回1，成功返回0
 */
int init() {
    // 清零源和目标数据结构
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    
    // 打印模型后处理配置信息
    printf(
        "post process config: box_conf_threshold = %.2f, nms_threshold = %.2f\n",
        box_conf_threshold, nms_threshold);
    
    // 创建神经网络模型
    printf("Loading mode...\n");
    
    // 加载模型数据
    model_data = load_model(model_name, &model_data_size);
    
    // 初始化RKNN上下文
    ret = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return 1;
    }
    
    // 查询并打印SDK版本信息
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version,
                     sizeof(rknn_sdk_version));
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return 1;
    }
    printf("sdk version: %s driver version: %s\n", version.api_version,
           version.drv_version);
    
    // 查询模型输入输出数量
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return 1;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input,
           io_num.n_output);
    
    // 初始化并查询输入属性
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]),
                         sizeof(rknn_tensor_attr));
        if (ret < 0) {
            printf("rknn_init error ret=%d\n", ret);
            return 1;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }
    
    // 初始化并查询输出属性
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]),
                         sizeof(rknn_tensor_attr));
        dump_tensor_attr(&(output_attrs[i]));
    }
    
    // 根据输入格式获取模型输入尺寸信息
    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
        printf("model is NCHW input fmt\n");
        channel = input_attrs[0].dims[1];
        height = input_attrs[0].dims[2];
        width = input_attrs[0].dims[3];
    } else {
        printf("model is NHWC input fmt\n");
        height = input_attrs[0].dims[1];
        width = input_attrs[0].dims[2];
        channel = input_attrs[0].dims[3];
    }
    printf("model input height=%d, width=%d, channel=%d\n", height, width,
           channel);
    
    // 初始化输入数据结构
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = width * height * channel;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].pass_through = 0;
}

std::string process(cv::Mat &orig_img) {
  cv::Mat img;
  cv::cvtColor(orig_img, img, cv::COLOR_BGR2RGB);
  img_width = img.cols;
  img_height = img.rows;
  printf("img width = %d, img height = %d\n", img_width, img_height);
  // 指定目标大小和预处理方式,默认使用LetterBox的预处理
  BOX_RECT pads;
  memset(&pads, 0, sizeof(BOX_RECT));
  cv::Size target_size(width, height);
  cv::Mat resized_img(target_size.height, target_size.width, CV_8UC3);
  // 计算缩放比例
  float scale_w = (float)target_size.width / img.cols;
  float scale_h = (float)target_size.height / img.rows;
  if (img_width != width || img_height != height) {
    printf("resize image with letterbox\n");
    float min_scale = std::min(scale_w, scale_h);
    scale_w = min_scale;
    scale_h = min_scale;
    letterbox(img, resized_img, pads, min_scale, target_size);
    // 保存预处理图片
    cv::imwrite("letterbox_input.jpg", resized_img);
    inputs[0].buf = resized_img.data;
  } else {
    inputs[0].buf = img.data;
  }
  gettimeofday(&start_time, NULL);
  rknn_inputs_set(ctx, io_num.n_input, inputs);
  rknn_output outputs[io_num.n_output];
  memset(outputs, 0, sizeof(outputs));
  for (int i = 0; i < io_num.n_output; i++) {
    outputs[i].index = i;
    outputs[i].want_float = 0;
  }
  // 执行推理
  ret = rknn_run(ctx, NULL);
  ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
  gettimeofday(&stop_time, NULL);
  printf("once run use %f ms\n",
         (__get_us(stop_time) - __get_us(start_time)) / 1000);
  // 后处理
  detect_result_group_t detect_result_group;
  std::vector<float> out_scales;
  std::vector<int32_t> out_zps;
  for (int i = 0; i < io_num.n_output; ++i) {
    out_scales.push_back(output_attrs[i].scale);
    out_zps.push_back(output_attrs[i].zp);
  }
  post_process((int8_t *)outputs[0].buf, (int8_t *)outputs[1].buf,
               (int8_t *)outputs[2].buf, height, width, box_conf_threshold,
               nms_threshold, pads, scale_w, scale_h, out_zps, out_scales,
               &detect_result_group);
  // 画框和概率
  char text[256];
  string result = "";
  for (int i = 0; i < detect_result_group.count; i++) {
    detect_result_t *det_result = &(detect_result_group.results[i]);
    sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);
    printf("%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left,
           det_result->box.top, det_result->box.right, det_result->box.bottom,
           det_result->prop);
    int x1 = det_result->box.left;
    int y1 = det_result->box.top;
    int x2 = det_result->box.right;
    int y2 = det_result->box.bottom;
    string name = "";
    name.assign(det_result->name);
    string temp = name + "&" + std::to_string(det_result->box.left) + "&" +
                  std::to_string(det_result->box.top) + "&" +
                  std::to_string(det_result->box.right) + "&" +
                  std::to_string(det_result->box.bottom) + "&" +
                  std::to_string(det_result->prop) + "@";
    result += temp;
  }
  ret = rknn_outputs_release(ctx, io_num.n_output, outputs);
  // 耗时统计
  //   int test_count = 10;
  //   gettimeofday(&start_time, NULL);
  //   for (int i = 0; i < test_count; ++i)
  //   {
  //     rknn_inputs_set(ctx, io_num.n_input, inputs);
  //     ret = rknn_run(ctx, NULL);
  //     ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
  // #if PERF_WITH_POST
  //     post_process((int8_t *)outputs[0].buf, (int8_t *)outputs[1].buf,
  //     (int8_t *)outputs[2].buf, height, width,
  //                  box_conf_threshold, nms_threshold, pads, scale_w, scale_h,
  //                  out_zps, out_scales, &detect_result_group);
  // #endif
  //     ret = rknn_outputs_release(ctx, io_num.n_output, outputs);
  //   }
  //   gettimeofday(&stop_time, NULL);
  //   printf("loop count = %d , average run  %f ms\n", test_count,
  //          (__get_us(stop_time) - __get_us(start_time)) / 1000.0 /
  //          test_count);
  //   deinitPostProcess();
  // release
  //   ret = rknn_destroy(ctx);
  //   if (model_data)
  //   {
  //     free(model_data);
  //   }
  return result;
}

int server_socket;
struct sockaddr_in server_address;
int client_socket;

void signal_handle(int signum) {
  // 关闭连接
  close(client_socket);
  close(server_socket);
  printf("Exit accidently!");
  ret = rknn_destroy(ctx);
  if (model_data) {
    free(model_data);
  }
  exit(signum);
}

int main() {
    // 注册信号处理函数
    signal(SIGINT, signal_handle);
    // init
    init();
    // 创建 socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
      std::cerr << "Error: Unable to create socket" << std::endl;
      return 1;
    }
    printf("create socket successfully!\n");
    // 绑定地址和端口
    string add = "127.0.0.1";
    int port = 12700;
    server_address.sin_family = AF_INET;
    // server_address.sin_addr.s_addr = inet_addr("192.168.50.59");
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    // 使用端口8080
    if (bind(server_socket, (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0) {
      std::cerr << "Error: Unable to bind address :" << errno << std::endl;
      return 1;
    }
    printf("bind server successfully in address:%s port:%d\n", add.c_str(), port);
    printf("waiting connection!\n");
    // 开始监听连接
    if (listen(server_socket, 1) == -1) {
      std::cerr << "Error: Unable to listen on socket" << std::endl;
      return 1;
    }
    printf("waiting connection!");

    // 接受连接
    client_socket = accept(server_socket, nullptr, nullptr);
    if (client_socket == -1) {
        std::cerr << "Error: Unable to accept connection" << std::endl;
        return 1;
    }
    printf("connected with a client!");

    // 打开摄像头
    cv::VideoCapture cap(21);
    if(!cap.isOpened()){
        std::cerr << "Error: Unable to open camera" << std::endl;
        return 1;
    }
    std::cout << "Camera opened successfully!" << std::endl;
    
    // while (true) {
    //     const int imgSize = 480 * 640 * 4;
    //     // 图像大小为 224x224x4
    //     char buffer[imgSize] = {0};
    //     int totalBytesReceived = 0;
    //     while (totalBytesReceived < imgSize) {
    //       int bytesReceived = read(client_socket, buffer + totalBytesReceived,
    //                               imgSize - totalBytesReceived);
    //       if (bytesReceived <= 0) {
    //         std::cerr << "Error receiving data, connection is closed!" << std::endl;
    //         close(client_socket);
    //         // 开始监听连接
    //         printf("waiting connection!");
    //         if (listen(server_socket, 1) == -1) {
    //           std::cerr << "Error: Unable to listen on socket" << std::endl;
    //           return 1;
    //         }

    //         // 接受连接
    //         client_socket = accept(server_socket, nullptr, nullptr);
    //         if (client_socket == -1) {
    //           std::cerr << "Error: Unable to accept connection" << std::endl;
    //           return 1;
    //         }
    //         break;
    //         // 结束当前连接，等待下一次连接
    //       }
    //       totalBytesReceived += bytesReceived;
    //     }
    //     if (totalBytesReceived == imgSize) {
    //       cv::Mat rgbImage;
    //       cv::Mat yuv(480 * 3 / 2, 640, CV_8UC1);
    //       yuv.data = (unsigned char *)(buffer);
    //       cv::cvtColor(yuv, rgbImage, COLOR_YUV420sp2BGR);

    //       // 转换为 RGB 图像
    //       std::cout << "input image size: " << rgbImage.rows * rgbImage.cols
    //                 << std::endl;
    //       string result = process(rgbImage);
    //       std::cout << "result:" << result << std::endl;
    //       const char *message = result.c_str();
    //       send(client_socket, message, strlen(message), 0);
    //     }
    // }
    cv::Mat frame;
    while (true) {
        cap >> frame;  // 抓取摄像头帧
        if (frame.empty()) {
            std::cerr << "Error: Frame capture failed" << std::endl;
            break;
        }

        // 压缩图像为JPEG格式
        std::vector<uchar> buffer;
        cv::imencode(".jpg", frame, buffer);
        int buffer_size = buffer.size();

        // 发送图像大小
        int total_bytes_sent = 0;
        total_bytes_sent = send(client_socket, &buffer_size, sizeof(buffer_size), 0);
        if (total_bytes_sent < 0) {
            std::cerr << "Error: Unable to send image size" << std::endl;
            break;
        }

        // 发送图像数据
        total_bytes_sent = 0;
        while (total_bytes_sent < buffer_size) {
            int bytes_sent = send(client_socket, buffer.data() + total_bytes_sent, buffer_size - total_bytes_sent, 0);
            if (bytes_sent < 0) {
                std::cerr << "Error: Unable to send image data" << std::endl;
                break;
            }
            total_bytes_sent += bytes_sent;
        }
    }
    // 发送数据给客户端
    const char *message = "Disconnected&0&100&0&100&0.9@";
    send(client_socket, message, strlen(message), 0);
    // 关闭连接
    close(client_socket);
    close(server_socket);
    return 0;
}