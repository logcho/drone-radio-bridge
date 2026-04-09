#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/empty.hpp>
#include <geometry_msgs/msg/twist.hpp>

class UAVTestNode : public rclcpp::Node {
public:
    UAVTestNode() : Node("uav_test_node") {
        sub_goto_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "goto", 10,
            std::bind(&UAVTestNode::goto_callback, this, std::placeholders::_1)
        );

        sub_land_ = this->create_subscription<std_msgs::msg::Empty>(
            "land", 10,
            std::bind(&UAVTestNode::land_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "UAV Test Node started. Listening to '/goto' and '/land'.");
    }

private:
    void goto_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "### RECEIVED GOTO COMMAND ### -> X: %.2f | Y: %.2f | Z: %.2f", 
            msg->linear.x, msg->linear.y, msg->linear.z);
    }

    void land_callback(const std_msgs::msg::Empty::SharedPtr /*msg*/) {
        RCLCPP_INFO(this->get_logger(), "### RECEIVED LAND COMMAND ### -> Initiating Landing Sequence.");
    }

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_goto_;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr sub_land_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<UAVTestNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
