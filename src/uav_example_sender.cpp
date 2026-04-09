#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/empty.hpp>

class UAVExampleSenderNode : public rclcpp::Node {
public:
    UAVExampleSenderNode() : Node("uav_example_sender"), counter_(0) {
        // Create publishers that talk to the parser node's inputs
        pub_goto_ = this->create_publisher<geometry_msgs::msg::Twist>("send_goto", 10);
        pub_land_ = this->create_publisher<std_msgs::msg::Empty>("send_land", 10);

        // Timer to simulate asynchronous commands coming from a drone's navigation stack
        timer_ = this->create_wall_timer(
            std::chrono::seconds(5),
            std::bind(&UAVExampleSenderNode::timer_callback, this)
        );

        RCLCPP_INFO(this->get_logger(), "Example Sender Node started.");
        RCLCPP_INFO(this->get_logger(), "Broadcasting mock coordinates to the parser every 5 seconds.");
    }

private:
    void timer_callback() {
        counter_++;

        // Every 3rd cycle (15 seconds), send a dummy land command
        if (counter_ % 3 == 0) {
            RCLCPP_INFO(this->get_logger(), "\n[Example Sender]: Triggering mock LAND command to '/send_land'");
            pub_land_->publish(std_msgs::msg::Empty());
        } else {
            // Otherwise, send goto coordinates
            auto msg = geometry_msgs::msg::Twist();
            msg.linear.x = 10.0 + counter_;
            msg.linear.y = 5.0 * counter_;
            msg.linear.z = 2.0;
            
            RCLCPP_INFO(this->get_logger(), "\n[Example Sender]: Publishing mock GOTO to '/send_goto': [%.2f, %.2f, %.2f]", 
                msg.linear.x, msg.linear.y, msg.linear.z);
            pub_goto_->publish(msg);
        }
    }

    int counter_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_goto_;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr pub_land_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<UAVExampleSenderNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
