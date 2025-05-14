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
#include <pthread.h>
#include <queue>

#define PORT 12700
#define CAMERA_FOV 60.0 // 摄像头视场角度
#define FOCUS_DISTANCE 500.0 // 焦距，单位为像素，需根据实际调整

// 共享数据结构
struct FrameData {
    cv::Mat frame;
    std::vector<cv::Rect> faces;
};

// 线程间通信队列
std::queue<FrameData> frame_queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

// 人脸检测线程函数
void* face_detection_thread(void* arg) {
    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load("//opencv/data/haarcascades/haarcascade_frontalface_default.xml")) {
        perror("Error loading face cascade");
        exit(EXIT_FAILURE);
    }

    cv::VideoCapture cap(21);
    if (!cap.isOpened()) {
        perror("Camera not opened");
        exit(EXIT_FAILURE);
    }

    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    cv::Mat frame, gray_frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray_frame, gray_frame);

        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(gray_frame, faces, 1.1, 3, 0, cv::Size(30, 30));

        // 将帧和人脸数据放入队列
        pthread_mutex_lock(&queue_mutex);
        frame_queue.push({frame, faces});
        pthread_cond_signal(&queue_cond);
        pthread_mutex_unlock(&queue_mutex);
    }

    cap.release();
    return nullptr;
}

// 帧发送线程函数
void* frame_send_thread(void* arg) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    while ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
        send(client_socket, "HTTP/1.1 200 OK\r\n"
                            "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n", 83, 0);

        while (true) {
            pthread_mutex_lock(&queue_mutex);
            while (frame_queue.empty()) {
                pthread_cond_wait(&queue_cond, &queue_mutex);
            }
            FrameData data = frame_queue.front();
            frame_queue.pop();
            pthread_mutex_unlock(&queue_mutex);

            cv::Mat frame = data.frame;
            std::vector<cv::Rect> faces = data.faces;

            for (const auto& face : faces) {
                cv::rectangle(frame, face, cv::Scalar(0, 0, 255), 2);
                cv::Point center(face.x + face.width / 2, face.y + face.height / 2);
                float face_width_cm = 15.0;
                float distance = (FOCUS_DISTANCE * face_width_cm) / face.width;
                float angle_x = atan2(center.y - frame.rows / 2, center.x - frame.cols / 2) * 180 / CV_PI;
                float angle_y = atan2(center.x - frame.cols / 2, center.y - frame.rows / 2) * 180 / CV_PI;

                putText(frame, "Distance: " + std::to_string((int)distance) + " cm, X: " + std::to_string((int)angle_x) + " degrees, Y: " + std::to_string((int)angle_y) + " degrees",
                        cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
            }

            std::vector<uchar> buffer;
            cv::imencode(".jpg", frame, buffer);
            int buffer_size = buffer.size();

            send(client_socket, "--frame\r\n", 10, 0);
            send(client_socket, "Content-Type: image/jpeg\r\n", 26, 0);
            char len[32];
            snprintf(len, sizeof(len), "Content-Length: %d\r\n\r\n", buffer_size);
            send(client_socket, len, strlen(len), 0);

            int bytes_sent = 0;
            while (bytes_sent < buffer_size) {
                int n = send(client_socket, buffer.data() + bytes_sent, buffer_size - bytes_sent, 0);
                if (n < 0) {
                    perror("Send failed");
                    break;
                }
                bytes_sent += n;
            }
            
            send(client_socket, "\r\n", 2, 0);
        }

        close(client_socket);
    }

    return nullptr;
}

int main() {
    pthread_t detection_thread, send_thread;

    // 创建人脸检测线程
    if (pthread_create(&detection_thread, NULL, face_detection_thread, NULL) != 0) {
        perror("Failed to create detection thread");
        exit(EXIT_FAILURE);
    }

    // 创建帧发送线程
    if (pthread_create(&send_thread, NULL, frame_send_thread, NULL) != 0) {
        perror("Failed to create send thread");
        exit(EXIT_FAILURE);
    }

    // 等待线程结束
    pthread_join(detection_thread, NULL);
    pthread_join(send_thread, NULL);

    return 0;
}