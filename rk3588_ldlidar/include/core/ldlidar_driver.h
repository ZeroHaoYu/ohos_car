#ifndef __LDLIDAR_NODE_H__
#define __LDLIDAR_NODE_H__

#include <chrono>
#include <functional>
#include <stdio.h>
#include <string>

#include "serial_interface_linux.h"
#include "lipkg.h"

namespace ldlidar {

typedef enum CommunicationMode {
  COMM_NO_NULL,
  COMM_SERIAL_MODE, /* 串口通信 */
  COMM_UDP_CLIENT_MODE, /* 网络通信（UDP客户端） */
  COMM_UDP_SERVER_MODE, /* 网络通信（UDP服务器） */
  COMM_TCP_CLIENT_MODE, /* 网络通信（TCP客户端） */
  COMM_TCP_SERVER_MODE  /* 网络通信（TCP服务器） */
} CommunicationModeTypeDef;

class LDLidarDriver {
 public:
  LDLidarDriver();
  ~LDLidarDriver();
  
  /**
   * @brief 获取激光雷达SDK版本号
   * @param 无
   * @return 字符串类型，例如 "v2.3.0"
  */
  std::string GetLidarSdkVersionNumber(void);

  /**
   * @brief 启动激光雷达设备处理节点
   * @param product_name
   *        激光雷达产品类型：ldlidar::LDType，值：
   *          - ldlidar::LDType::NOVER
   *          - ldlidar::LDType::LD_06
   *            ....
   *        - 其他定义在 "ldlidar_driver/include/ldlidar_datatype.h"
   * @param serial_port_name
   *        串口设备系统路径，例如："/dev/ttyUSB0"
   * @param serial_baudrate
   *        串口通信波特率值（> 0），单位是位/秒。
   * @param comm_mode
   *        通信模式，默认为串口通信模式
   * @return 如果值为 true，启动成功；
   *         如果值为 false，启动失败。
  */
  bool Start(LDType product_name, 
            std::string serial_port_name, 
            uint32_t serial_baudrate = 115200,
            CommunicationModeTypeDef comm_mode = COMM_SERIAL_MODE);

  // bool Start(LDType product_name, 
  //           const char* server_ip, 
  //           const char* server_port,
  //           CommunicationModeTypeDef comm_mode = COMM_TCP_CLIENT_MODE);

  /**
   * @brief 停止激光雷达设备处理节点
   * @param 无
   * @return 如果值为 true，停止成功；
   *         如果值为 false，停止失败。
  */
  bool Stop(void);


  /**
   * @brief 激光雷达上电后，检查通信通道连接是否正常
   * @param timeout: 等待超时时间，单位是毫秒
   * @return 如果 times >= timeout，返回 false，通信连接失败；
   *         如果 "times < timeout"，返回 true，通信连接成功。
  */
  bool WaitLidarCommConnect(int64_t timeout);

  /**
   * @brief 获取激光雷达激光扫描点云数据
   * @param [输出]
   * @param dst: 类型是 ldlidar::Point2D，值是激光扫描点云数据
   * @param [输入]
   * @param timeout: 等待超时时间，单位是毫秒
   * @return 返回值是 ldlidar::LidarStatus 枚举类型，定义在 "ldlidar_driver/include/ldlidar_datatype.h"，值是：
   *  ldlidar::LidarStatus::NORMAL   // 激光雷达正常
   *  ldlidar::LidarStatus::ERROR    // 激光雷达异常错误
   *  ....
  */
  LidarStatus GetLaserScanData(Points2D& dst, int64_t timeout = 1000);

  LidarStatus GetLaserScanData(LaserScan& dst, int64_t timeout = 1000);

  /**
   * @brief 获取激光雷达激光扫描频率
   * @param [输出]
   * @param spin_hz: 单位是 Hz。
   * @return 如果值为 true，获取激光雷达激光扫描频率成功；
   *         如果值为 false，获取激光雷达激光扫描频率失败。
  */
  bool GetLidarScanFreq(double& spin_hz);

  /**
   * @brief 注册获取时间戳的处理函数。
   * @param [输入]
   * @param get_timestamp_handle: 类型是 `uint64_t (*)(void)`，值是指向获取时间戳函数的指针。
   * @return 无
  */
  void RegisterGetTimestampFunctional(std::function<uint64_t(void)> get_timestamp_handle);

  /**
   * @brief 打开或关闭滤波算法处理
   * @param [输入]
   * @param is_enable:  值为 true，打开滤波处理；
   *         值为 false，关闭滤波处理。
   * @return 无
  */
  void EnableFilterAlgorithnmProcess(bool is_enable);

  /**
   * @brief 当激光雷达处于错误状态时，获取激光雷达反馈的错误代码
   * @param 无
   * @return 错误代码
  */
  uint8_t GetLidarErrorCode(void);

  static bool IsOk() { return is_ok_; }

  static void SetIsOkStatus(bool status) { is_ok_ = status;}
  
 private:
  std::string sdk_version_number_;
  static bool is_ok_;
  bool is_start_flag_;
  std::function<uint64_t(void)> register_get_timestamp_handle_;
  LiPkg* comm_pkg_;
  SerialInterfaceLinux* comm_serial_;
  std::chrono::steady_clock::time_point last_pubdata_times_;
};

} // namespace ldlidar

#endif // __LDLIDAR_DRIVER_H__