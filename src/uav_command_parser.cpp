#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/empty.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <string>
#include <sstream>

class UAVCommandParserNode : public rclcpp::Node {
public:
    UAVCommandParserNode() : Node("uav_command_parser") {
        /* ========================================================================= 
         * 1. PUBLISHERS: RECEIVING FROM RADIO -> SENDING TO LOCAL ROS NETWORK 
         * =========================================================================
         * If you want to add a new command type (like TAKEOFF or ARM):
         * - Declare a publisher here (e.g., pub_takeoff_ = this->create_publisher...)
         * - Add the publisher to the private class variables at the bottom of the file
         */
        pub_goto_ = this->create_publisher<geometry_msgs::msg::Twist>("goto", 10);
        pub_land_ = this->create_publisher<std_msgs::msg::Empty>("land", 10);
        pub_start_ = this->create_publisher<std_msgs::msg::Empty>("start", 10);

        // Publisher to transmit strings out over the raw radio interface
        pub_radio_tx_ = this->create_publisher<std_msgs::msg::String>("radio_tx", 10);

        /* ========================================================================= 
         * 2. SUBSCRIBERS: RECEIVING FROM LOCAL ROS NETWORK -> SENDING OUT OVER RADIO
         * =========================================================================
         * If you want to serialize a new command into a string, create a subscription
         * to a new "send_X" topic down below, and bind it to a new callback function.
         */
        sub_radio_rx_ = this->create_subscription<std_msgs::msg::String>(
            "radio_rx", 10,
            std::bind(&UAVCommandParserNode::radio_rx_callback, this, std::placeholders::_1)
        );

        sub_send_goto_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "send_goto", 10,
            std::bind(&UAVCommandParserNode::send_goto_callback, this, std::placeholders::_1)
        );
        
        sub_send_land_ = this->create_subscription<std_msgs::msg::Empty>(
            "send_land", 10,
            std::bind(&UAVCommandParserNode::send_land_callback, this, std::placeholders::_1)
        );

        sub_send_start_ = this->create_subscription<std_msgs::msg::Empty>(
            "send_start", 10,
            std::bind(&UAVCommandParserNode::send_start_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "UAV Command Parser started.");
    }

private:
    /* ========================================================================= 
     * PARSING INCOMING STRINGS FROM THE RADIO 
     * =========================================================================
     * This function listens to the radio bridge. 
     * To add custom string commands (e.g. CMD:TAKEOFF):
     * - Add an 'else if (data.rfind("CMD:TAKEOFF", 0) == 0)' block below.
     * - Inside that block, trigger your new custom publisher! (pub_takeoff_)
     */
    void radio_rx_callback(const std_msgs::msg::String::SharedPtr msg) {
        std::string data = msg->data;
        
        if (data.rfind("CMD:LAND", 0) == 0) {
            RCLCPP_INFO(this->get_logger(), "Parsed LAND over radio. Triggering '/land'.");
            pub_land_->publish(std_msgs::msg::Empty());
        } 
        else if (data.rfind("GOTO:", 0) == 0) {
            try {
                // Trim standard "GOTO:" and parse coordinates
                std::string payload = data.substr(5);
                std::stringstream ss(payload);
                std::string token;
                
                float x = 0.0, y = 0.0, z = 0.0;
                if (std::getline(ss, token, ',')) x = std::stof(token);
                if (std::getline(ss, token, ',')) y = std::stof(token);
                if (std::getline(ss, token, ',')) z = std::stof(token);

                auto twist_msg = geometry_msgs::msg::Twist();
                twist_msg.linear.x = x;
                twist_msg.linear.y = y;
                twist_msg.linear.z = z;
                
                // This publishes the native Twist message directly to the ROS2 '/goto' topic!
                // Any native drone navigational node can now subscribe to '/goto' to receive it seamlessly.
                pub_goto_->publish(twist_msg);
                RCLCPP_INFO(this->get_logger(), "Parsed GOTO over radio: [%.2f, %.2f, %.2f]. Triggering '/goto'.", x, y, z);
            } catch (const std::exception& e) {
                RCLCPP_ERROR(this->get_logger(), "Failed to parse GOTO packet coordinates: %s", e.what());
            }
        }
        else if (data.rfind("CMD:START", 0) == 0) {
            RCLCPP_INFO(this->get_logger(), "Parsed START over radio. Triggering '/start'.");
            pub_start_->publish(std_msgs::msg::Empty());
        }
        else {
            RCLCPP_WARN(this->get_logger(), "Received unknown string format: %s", data.c_str());
        }
    }

    /* ========================================================================= 
     * SERIALIZING OUTGOING MESSAGES TO THE RADIO
     * =========================================================================
     * These callbacks capture your native ROS messages and format them as 
     * strings to pass over the hardware bridge.
     * To add custom strings, copy one of these blocks, rename it, and change 
     * the 'out_msg.data' format to whatever string structure you want.
     */
    void send_goto_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
        auto out_msg = std_msgs::msg::String();
        std::stringstream ss;
        // Float conversion to simple string via stringstream
        ss << "GOTO:" << msg->linear.x << "," << msg->linear.y << "," << msg->linear.z;
        out_msg.data = ss.str();
        
        pub_radio_tx_->publish(out_msg);
        RCLCPP_INFO(this->get_logger(), "Sending GOTO over radio: %s", out_msg.data.c_str());
    }

    void send_land_callback(const std_msgs::msg::Empty::SharedPtr /*msg*/) {
        auto out_msg = std_msgs::msg::String();
        out_msg.data = "CMD:LAND";
        
        pub_radio_tx_->publish(out_msg);
        RCLCPP_INFO(this->get_logger(), "Sending LAND over radio.");
    }

    // --- Private Variables (Add new publishers and subscriptions here!) ---
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_goto_;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr pub_land_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_radio_tx_;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_radio_rx_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_send_goto_;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr sub_send_land_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<UAVCommandParserNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
