#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/empty.hpp>
#include <iostream>
#include <string>
#include <sstream>

class UAVInteractiveSenderNode : public rclcpp::Node {
public:
    UAVInteractiveSenderNode() : Node("uav_interactive_sender") {
        pub_goto_ = this->create_publisher<geometry_msgs::msg::Twist>("send_goto", 10);
        pub_land_ = this->create_publisher<std_msgs::msg::Empty>("send_land", 10);
    }

    void process_input(const std::string& input) {
        if (input == "land" || input == "LAND") {
            RCLCPP_INFO(this->get_logger(), "Sending native LAND payload -> '/send_land'");
            pub_land_->publish(std_msgs::msg::Empty());
        } else {
            // Replace commas with spaces to handle both "10.0,5.0,2.0" and "10.0 5.0 2.0" formats
            std::string cleaned_input = input;
            for (char& c : cleaned_input) {
                if (c == ',') c = ' ';
            }

            std::stringstream ss(cleaned_input);
            float x, y, z;
            if (ss >> x >> y >> z) {
                auto msg = geometry_msgs::msg::Twist();
                msg.linear.x = x;
                msg.linear.y = y;
                msg.linear.z = z;
                RCLCPP_INFO(this->get_logger(), "Sending native Twist GOTO payload -> '/send_goto': [%.2f, %.2f, %.2f]", x, y, z);
                pub_goto_->publish(msg);
            } else {
                std::cout << "[ERROR] Invalid format! Please type 'land' OR three coordinates (e.g. '10,5,2').\n" << std::endl;
            }
        }
    }

private:
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_goto_;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr pub_land_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<UAVInteractiveSenderNode>();

    std::cout << "\n============================================\n";
    std::cout << " UAV Interactive Mock Sender\n";
    std::cout << "============================================\n";
    std::cout << " Commands:\n";
    std::cout << "  - 'land'               : triggers emergency land\n";
    std::cout << "  - 'x,y,z' or 'x y z'   : triggers native GOTO command\n";
    std::cout << "  - 'quit'               : close terminal\n";
    std::cout << "============================================\n\n";

    std::string line;
    // We run the input loop entirely in main since this node doesn't subscribe to anything.
    while (rclcpp::ok()) {
        std::cout << ">> ";
        if (!std::getline(std::cin, line)) {
            break; // EOF caught
        }

        if (line == "exit" || line == "quit" || line == "q") {
            break;
        }

        if (!line.empty()) {
            node->process_input(line);
        }
    }

    rclcpp::shutdown();
    return 0;
}
