// ROS2 wrapper for ugv-radio-bridge
// This file will ONLY be built on the Jetson Nano
// when ROS2 is available.
//
// It will:
// - subscribe to /cmd_vel
// - call Encoder::encodeCmdVel()
// - write to SerialPort
// - read from SerialPort
// - feed PacketParser
// - publish decoded packets

#ifdef BUILD_ROS2

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

#include "serial_port.hpp"
#include "packet_parser.hpp"
#include "packet_encoder.hpp"

using namespace std::chrono_literals;

class RadioBridgeNode : public rclcpp::Node {
public:
  RadioBridgeNode()
  : Node("radio_bridge_node"),
    serial_("/dev/ttyUSB0", 57600)
  {
    if (!serial_.openPort()) {
      RCLCPP_FATAL(get_logger(), "Failed to open serial port: %s at %d baud", 
                   "/dev/ttyUSB0", 57600);
      throw std::runtime_error("Serial port initialization failed");
    } else {
      RCLCPP_INFO(get_logger(), "Serial port opened on /dev/ttyUSB0 at 57600 baud");
    }

    cmd_vel_sub_ = create_subscription<geometry_msgs::msg::Twist>(
      "/cmd_vel", 10,
      std::bind(&RadioBridgeNode::onCmdVel, this, std::placeholders::_1)
    );

    poll_timer_ = create_wall_timer(
      10ms, std::bind(&RadioBridgeNode::pollSerial, this)
    );
  }

private:
  SerialPort serial_;
  PacketParser parser_;
  uint8_t seq_{0};

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::TimerBase::SharedPtr poll_timer_;

  void onCmdVel(const geometry_msgs::msg::Twist::SharedPtr msg) {
    auto pkt = Encoder::encodeCmdVel(
      seq_++,
      (float)msg->linear.x,
      (float)msg->angular.z
    );
    int written = serial_.writeBytes(pkt.data(), pkt.size());
    if (written < 0) {
      RCLCPP_ERROR(get_logger(), "Failed to write %zu bytes to serial port", pkt.size());
    } else if ((size_t)written < pkt.size()) {
      RCLCPP_WARN(get_logger(), "Partial write: %d of %zu bytes", written, pkt.size());
    } else {
      RCLCPP_DEBUG(get_logger(), "TX seq=%u %zu bytes", seq_ - 1, pkt.size());
    }
  }

  void pollSerial() {
    uint8_t buf[256];
    int n = serial_.readBytes(buf, sizeof(buf));
    if (n < 0) {
      RCLCPP_ERROR(get_logger(), "Serial read error");
      return;
    }
    if (n == 0) return;

    RCLCPP_DEBUG(get_logger(), "RX %d raw bytes", n);

    auto packets = parser_.feed({buf, buf + n});
    for (auto &p : packets) {
      RCLCPP_INFO(
        get_logger(),
        "RX msg_id=%u seq=%u payload=%zu",
        p.msg_id, p.seq, p.payload.size()
      );
    }
  }
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RadioBridgeNode>());
  rclcpp::shutdown();
  return 0;
}

#endif // BUILD_ROS2
