#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cmath>
#include <opencv2/objdetect.hpp>
#include <unistd.h>
#include <vector>

#define PORT 8080
#define CAMERA_FOV 60.0 // 摄像头视场角度
#define FOCUS_DISTANCE 500.0 // 焦距，单位为像素，需根据实际调整
int main() {
    // 加载Haar级联分类器
    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load("/opencv/data/haarcascades/haarcascade_frontalface_default.xml")) {
        perror("Error loading face cascade");
        exit(EXIT_FAILURE);
        return -1;
    }
    
    // 打开默认摄像头
    cv::VideoCapture cap(21);  // 设备编号为21，确保摄像头能正确打开
    if (!cap.isOpened()) {
        perror("Camera not opened");
        exit(EXIT_FAILURE);
        return -1;
    }
    // 设置分辨率,可选，注释掉后为默认配置
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);  // 设置宽度
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);   // 设置高度

    printf("Camera opened\n");

    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 创建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created\n");

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定Socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }
    printf("Binding port %d\n", PORT);

    // 监听客户端连接
    if (listen(server_fd, 3) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connection...\n");

    // 主循环等待客户端连接
    while ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
        printf("Client connected\n");

        // 发送一次HTTP响应头
        send(client_socket, "HTTP/1.1 200 OK\r\n"
                            "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n", 83, 0);

        cv::Mat frame, gray_frame;

        while (true) {
            cap >> frame; // 捕获帧
            if (frame.empty()) break;

            // 转换为灰度图像以便于人脸检测
            cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray_frame, gray_frame); // 增强对比度以提高检测效果

            // 检测人脸
            std::vector<cv::Rect> faces;
            face_cascade.detectMultiScale(gray_frame, faces, 1.1, 3, 0, cv::Size(30, 30));
            
            // 在图像中绘制检测到的人脸矩形框
            for (const auto& face : faces) {
                cv::rectangle(frame, face, cv::Scalar(0, 0, 255), 2);
                // 计算人脸中心点
                cv::Point center(face.x + face.width / 2, face.y + face.height / 2);

                // 计算距离
                float face_width_cm = 15.0; // 假设人脸宽度为15厘米
                float distance = (FOCUS_DISTANCE * face_width_cm) / face.width;

                // 计算角度
                float angle_x = atan2(center.y - frame.rows / 2, center.x - frame.cols / 2) * 180 / CV_PI; // Y轴
                float angle_y = atan2(center.x - frame.cols / 2, center.y - frame.rows / 2) * 180 / CV_PI; // X轴

                // 在图像上显示距离和坐标
                putText(frame, "Distance: " + std::to_string((int)distance) + " cm, X: " + std::to_string((int)angle_x) + " degrees, Y: " + std::to_string((int)angle_y) + " degrees",
                        cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
            }

            // 编码帧为 JPEG
            std::vector<uchar> buffer;
            cv::imencode(".jpg", frame, buffer);
            int buffer_size = buffer.size();

            // 发送帧的边界标记
            send(client_socket, "--frame\r\n", 10, 0);
            send(client_socket, "Content-Type: image/jpeg\r\n", 26, 0);

            // 发送内容长度
            char len[32];
            snprintf(len, sizeof(len), "Content-Length: %d\r\n\r\n", buffer_size);
            send(client_socket, len, strlen(len), 0);

            // 发送JPEG数据
            int bytes_sent = 0;
            while (bytes_sent < buffer_size) {
                int n = send(client_socket, buffer.data() + bytes_sent, buffer_size - bytes_sent, 0);
                if (n < 0) {
                    perror("Send failed");
                    break;
                }
                bytes_sent += n;
            }

            // 发送换行符
            send(client_socket, "\r\n", 2, 0);
        }

        close(client_socket);
        cap.release();
    }
    return 0;
}
