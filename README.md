# UGV Radio Bridge

A bare-bones ROS2 node to bridge string-based communications over a serial radio connection.

## Overview
This package is designed for extreme simplicity, stripping away complex packet parsers in favor of standard raw text bridging. It establishes a connection over a specified serial port and implements bidirectional string bridging:
- It listens for incoming `std_msgs/msg/String` messages on the `radio_tx` ROS2 topic, and writes them line-by-line over the active serial port.
- It actively polls the incoming serial transmission, constructs strings by recognizing newline (`\n`) characters, and publishes those internal strings to the `radio_rx` ROS2 topic.

## Prerequisites
- **ROS 2** (Foxy, Humble, Jazzy, etc.)
- A connected serial radio link (SiK Telemetry, RFD900, XBee, etc.)

## Building
Run the following from the root of your ROS2 workspace:
```bash
colcon build --packages-select ugv_radio_bridge
source install/setup.bash
```

## Running the Node
Before starting the node, you should ensure your user has the correct permissions to access the serial port. You can grant full access with:
```bash
sudo chmod 777 /dev/ttyUSB0
```

Start the ROS2 node by executing:
```bash
ros2 run ugv_radio_bridge radio_bridge_node
```

### Specifying the Serial Port
By default, the node uses `/dev/ttyUSB0` (Standard Linux format). Ensure you change this to the actual port where your radio is physically located if it differs. 

You can pass the serial port dynamically using `--ros-args`:
**Linux/Ubuntu:**
```bash
ros2 run ugv_radio_bridge radio_bridge_node --ros-args -p serial_port:=/dev/ttyUSB0
```
**Mac/macOS:**
```bash
ros2 run ugv_radio_bridge radio_bridge_node --ros-args -p serial_port:=/dev/cu.usbserial-A900XXXX
```

## System Architecture & Running the Nodes

To run the complete hardware-to-software architecture, you should spin up the base serial radio and the high-level ROS translator in tandem.

**1. Start the Base Radio Bridge**
```bash
# This handles the raw serial hardware link
ros2 run ugv_radio_bridge radio_bridge_node
```

**2. Start the UAV Command Parser**
```bash
# This translates the raw radio strings into native ROS2 geometry/command messages
ros2 run ugv_radio_bridge uav_command_parser_node
```

## How to Listen and Test (The Test Node)

We have provided a dedicated mockup tool called `uav_test_node`. It subscribes directly to the native `/goto` and `/land` outputs produced by the parser and logs them neatly to the terminal console so you can verify that packets are being received correctly.

To start listening, open a new terminal and run:
```bash
ros2 run ugv_radio_bridge uav_test_node
```
Leave it running! If coordinates or land commands come through the radio from a remote UGV/UAV, they will print out here cleanly.

## How to Autonomously Emit Data (The Example Sender)

If you'd like to simulate an onboard flight controller publishing live coordinates out to the bridge automatically without having to type terminal commands manually, we've provided a boilerplate sender node! 

To run it, open a new terminal:
```bash
ros2 run ugv_radio_bridge uav_example_sender_node
```
*(This node automatically generates and publishes `Twist` structures every 5 seconds, and an `Empty` landing payload every 15 seconds straight into the parser pipeline!)*

## Live Interactive Testing

If you don't want to use standard `ros2 topic pub` commands, you can use the built-in interactive terminal mockup!
```bash
ros2 run ugv_radio_bridge uav_interactive_sender_node
```
This spawns a clean `>>` prompt where you can simply type `land` or coordinates like `10.5,5.2,2.0` and hit enter. It takes your raw strings, generates the native ROS Twist/Empty structures, and fires them into your parser framework seamlessly.

## How to Send Coordinates and Commands

Instead of dealing with raw strings yourself, you can now natively send structured ROS messages to the parser, which will serialize them properly and trigger the radio transmission automatically.

### 1. Send Coordinates (GOTO)
Publish a `geometry_msgs/msg/Twist` message to the `/send_goto` topic:
```bash
ros2 topic pub --once /send_goto geometry_msgs/msg/Twist "{linear: {x: 10.0, y: 5.0, z: 2.0}}"
```
*The parser node will catch this, parse it into `GOTO:10.0,5.0,2.0`, and transmit it over the serial link.*

### 2. Send the Land Command
Publish an `std_msgs/msg/Empty` message to the `/send_land` topic:
```bash
ros2 topic pub --once /send_land std_msgs/msg/Empty "{}"
```
*The parser will catch this and transmit `CMD:LAND` to the remote radios.*

*(Note: if you instead want to test the raw string connection directly without the geometry parsing overhead, you can still publish raw strings directly to `/radio_tx` and echo `/radio_rx`)*

## Integrating With Your Own Nodes (C++ Example)

If you are writing your own flight controller or navigation node and want to bridge it to this radio network natively, you don't need to deal with strings! Simply create standard ROS2 Publishers and Subscriptions to pass data into the parsing pipeline.

**To Transmit Data Over Radio:**
Publish native ROS types to `/send_goto` and `/send_land`:
```cpp
// 1. Include the libraries
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/empty.hpp>

// 2. Create Publishers in your node's constructor
auto pub_goto = this->create_publisher<geometry_msgs::msg::Twist>("send_goto", 10);
auto pub_land = this->create_publisher<std_msgs::msg::Empty>("send_land", 10);

// 3. Publish when ready! The parser serializes this and shoots it over the bridge.
auto msg = geometry_msgs::msg::Twist();
msg.linear.x = 10.0;
pub_goto->publish(msg);
```

**To Receive Data From Radio:**
Create subscriptions that listen to `/goto` and `/land`:
```cpp
// Create Subscriptions in your node's constructor
auto sub_goto = this->create_subscription<geometry_msgs::msg::Twist>(
    "goto", 10,
    [](const geometry_msgs::msg::Twist::SharedPtr msg) {
        // Remote UGV mapped Twist coordinates over the radio! 
        // Feed msg->linear.x/y/z to your flight loop here.
    }
);

auto sub_land = this->create_subscription<std_msgs::msg::Empty>(
    "land", 10,
    [](const std_msgs::msg::Empty::SharedPtr msg) {
        // Remote UGV sent LAND command over the radio! Trigger sequence.
    }
);
```

## Additional Information
- **Baud Rate:** Defaults to `57600`, which is the standard for most telemetry radios. You can override this using the `baud_rate` ROS parameter:
  ```bash
  ros2 run ugv_radio_bridge radio_bridge_node --ros-args -p serial_port:=/dev/cu... -p baud_rate:=115200
  ```
- **Data Formatting:** The string parsing inherently expects new lines line-terminating characters (`\n`). If trailing carriage returns (`\r`) exist from the transmitting computer, they are stripped automatically prior to being sent via standard ROS2 logs.
- **Non-blocking Flow:** There is no hardware flow control overhead implemented here (RTS/CTS options are actively removed in code). It's uniquely built for lightweight telemetry streams.
