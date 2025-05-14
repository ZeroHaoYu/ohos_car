#include "ldlidar_driver.h"

uint64_t GetSystemTimeStamp(void) {
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tp = 
    std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
  auto tmp = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
  return ((uint64_t)tmp.count());
}

int main(int argc, char **argv) {

  std::string product_name = "LD09";
  std::string communication_mode = "serialcom";
  std::string port_name = argv[1];
  uint32_t serial_baudrate = 230400;
  ldlidar::LDType type_name = ldlidar::LDType::LD_19;

  // 创建激光雷达驱动对象
  ldlidar::LDLidarDriver* node = new ldlidar::LDLidarDriver();

  // 输出SDK版本信息
  printf("LDLiDAR SDK Pack Version is %s", node->GetLidarSdkVersionNumber().c_str());

  // 注册获取时间戳的函数
  node->RegisterGetTimestampFunctional(std::bind(&GetSystemTimeStamp)); 

  // 启用数据过滤算法处理
  node->EnableFilterAlgorithnmProcess(true);

  // 启动雷达
  if (node->Start(type_name, port_name, serial_baudrate, ldlidar::COMM_SERIAL_MODE)) {
    printf("ldlidar node start is success\r\n");
    printf("Attempting to connect to lidar on %s at baud rate %d", port_name.c_str(), serial_baudrate);
    // LidarPowerOn();
  } else {
    printf("ldlidar node start is fail\r\n");
    exit(EXIT_FAILURE);
  }
  // 等待雷达通信连接建立
  if (node->WaitLidarCommConnect(3500)) {
    printf("ldlidar communication is normal.\r\n");
  } else {
    printf("ldlidar communication is abnormal.\r\n");
    node->Stop();
  }
  // 创建激光雷达扫描点云数据对象
  ldlidar::Points2D laser_scan_points;
  
  // 主循环，获取并处理激光雷达扫描数据
  while (ldlidar::LDLidarDriver::IsOk()) {

    switch (node->GetLaserScanData(laser_scan_points, 1500)){
      case ldlidar::LidarStatus::NORMAL: {
        double lidar_scan_freq = 0;
        node->GetLidarScanFreq(lidar_scan_freq);

#ifdef __LP64__
        printf("speed(Hz):%f, size:%lu, stamp_front:%lu,  stamp_back:%lu\r\n",
          lidar_scan_freq, laser_scan_points.size(), laser_scan_points.front().stamp, laser_scan_points.back().stamp);
#else
        printf("speed(Hz):%f,size:%lu,stamp_front:%llu, stamp_back:%llu",
          lidar_scan_freq, laser_scan_points.size(), laser_scan_points.front().stamp, laser_scan_points.back().stamp);
#endif

        // 输出2D点云数据
        for (auto point : laser_scan_points) {
#ifdef __LP64__
          printf("angle:%0.3f,  distance(cm):%04d,  intensity:%d\r\n", point.angle, point.distance, point.intensity);
#else
          printf("angle:%f,distance(mm):%d,intensity:%d", 
            point.angle, point.distance, point.intensity);
#endif
        }
        
        break;
      }
      case ldlidar::LidarStatus::DATA_TIME_OUT: {
        printf("ldlidar publish data is time out, please check your lidar device.");
        node->Stop();
        break;
      }
      case ldlidar::LidarStatus::DATA_WAIT: {
        break;
      }
      default: {
        break;
      }
    }

    usleep(1000 * 100);  // sleep 100ms  == 10Hz
  }

  node->Stop();
  // LidarPowerOff();

  delete node;
  node = nullptr;

  return 0;
}