#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string>

class RadioBridgeNode : public rclcpp::Node {
public:
    RadioBridgeNode() : Node("radio_bridge"), serial_fd_(-1) {
        // Declare and get the serial port parameter
        this->declare_parameter<std::string>("serial_port", "/dev/cu.usbserial-00000000"); // Standard mac serial format placeholder
        std::string port_name = this->get_parameter("serial_port").as_string();

        // Publisher for strings received FROM the radio
        publisher_ = this->create_publisher<std_msgs::msg::String>("radio_rx", 10);

        // Subscriber for strings to send TO the radio
        subscriber_ = this->create_subscription<std_msgs::msg::String>(
            "radio_tx", 10,
            std::bind(&RadioBridgeNode::tx_callback, this, std::placeholders::_1)
        );

        // Initialize Serial Port
        if (init_serial(port_name)) {
            RCLCPP_INFO(this->get_logger(), "Serial port %s opened successfully.", port_name.c_str());
            
            // Timer for checking incoming data from serial at 100Hz
            timer_ = this->create_wall_timer(
                std::chrono::milliseconds(10), 
                std::bind(&RadioBridgeNode::poll_radio, this)
            );
        } else {
            RCLCPP_ERROR(this->get_logger(), "Failed to open serial port %s. Publishing will not work without it.", port_name.c_str());
            RCLCPP_INFO(this->get_logger(), "To specify a port, use: ros2 run ugv_radio_bridge radio_bridge_node --ros-args -p serial_port:=/dev/ttyUSB0");
        }

        RCLCPP_INFO(this->get_logger(), "Started barebones string-based radio bridge.");
        RCLCPP_INFO(this->get_logger(), "Subscribed to 'radio_tx', publishing to 'radio_rx'");
    }

    ~RadioBridgeNode() {
        if (serial_fd_ >= 0) {
            close(serial_fd_);
        }
    }

private:
    bool init_serial(const std::string& port_name) {
        serial_fd_ = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (serial_fd_ < 0) {
            return false;
        }

        struct termios tty;
        if (tcgetattr(serial_fd_, &tty) != 0) {
            return false;
        }

        // Set Baud Rate to 115200 (standard for many radios like SiK Telemetry)
        cfsetospeed(&tty, B115200);
        cfsetispeed(&tty, B115200);

        tty.c_cflag |= (CLOCAL | CREAD);    // Enable receiver, ignore modem control lines
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;                 // 8 data bits
        tty.c_cflag &= ~PARENB;             // No parity
        tty.c_cflag &= ~CSTOPB;             // 1 stop bit
        tty.c_cflag &= ~CRTSCTS;            // Disable hardware flow control

        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
        tty.c_oflag &= ~OPOST;                          // Raw output

        tty.c_cc[VMIN]  = 0; // Non-blocking read
        tty.c_cc[VTIME] = 0; // 0 second read timeout

        if (tcsetattr(serial_fd_, TCSANOW, &tty) != 0) {
            return false;
        }

        return true;
    }

    void tx_callback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Sending string over radio: '%s'", msg->data.c_str());
        
        if (serial_fd_ >= 0) {
            // Write string and append newline for simple delimiting
            std::string data_to_send = msg->data + "\n";
            write(serial_fd_, data_to_send.c_str(), data_to_send.length());
        } else {
            RCLCPP_WARN(this->get_logger(), "Cannot send, serial port is not open.");
        }
    }

    void poll_radio() {
        if (serial_fd_ < 0) return;

        char buf[256];
        // Read without blocking
        int n = read(serial_fd_, buf, sizeof(buf) - 1);
        
        if (n > 0) {
            buf[n] = '\0';
            rx_buffer_ += buf;

            // Process line by line
            size_t pos;
            while ((pos = rx_buffer_.find('\n')) != std::string::npos) {
                std::string line = rx_buffer_.substr(0, pos);
                rx_buffer_.erase(0, pos + 1);

                // Ignore carriage returns if present
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                if (!line.empty()) {
                    auto msg = std_msgs::msg::String();
                    msg.data = line;
                    publisher_->publish(msg);
                    RCLCPP_INFO(this->get_logger(), "Received string over radio: '%s'", line.c_str());
                }
            }
        }
    }

    int serial_fd_;
    std::string rx_buffer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RadioBridgeNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
