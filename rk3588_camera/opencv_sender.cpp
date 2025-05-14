#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12500
#define FOCUS_DISTANCE 500  // 假设焦距为500mm

struct FrameData {
    cv::Mat frame;
    std::vector<cv::Rect> faces;
};

void process_frame_and_send(cv::Mat& frame, std::vector<cv::Rect>& faces, int client_socket) {
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

int main() {
    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load("//opencv/data/haarcascades/haarcascade_frontalface_default.xml")) {
        perror("Error loading face cascade");
        return EXIT_FAILURE;
    }

    cv::VideoCapture cap(21);
    if (!cap.isOpened()) {
        perror("Camera not opened");
        return EXIT_FAILURE;
    }

    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding failed");
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listening failed");
        return EXIT_FAILURE;
    }

    while ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
        send(client_socket, "HTTP/1.1 200 OK\r\n"
                            "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n", 83, 0);

        cv::Mat frame, gray_frame;
        while (true) {
            cap >> frame;
            if (frame.empty()) break;

            cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray_frame, gray_frame);

            std::vector<cv::Rect> faces;
            face_cascade.detectMultiScale(gray_frame, faces, 1.1, 3, 0, cv::Size(30, 30));

            if (faces.empty()) {
                printf("No faces detected\n");
            } else {
                printf("Detected %zu faces\n", faces.size());
            }

            process_frame_and_send(frame, faces, client_socket);
        }

        close(client_socket);
    }

    cap.release();
    return 0;
}