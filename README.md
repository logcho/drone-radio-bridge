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

## How to Test and Send Messages

Once the node is active, you can send strings to the remote radio by publishing to the `radio_tx` topic. 

### 1. Send an Outgoing Radio Message
Use the standard ROS2 publishing command to write out to the serial interface:
```bash
ros2 topic pub --once /radio_tx std_msgs/msg/String '{data: "Hello Radio World!"}'
```
*Note: A newline `\n` is automatically added to the end of the data being fired out to the serial port.*

### 2. Listen for Incoming Radio Messages
To monitor strings coming from the radio, you can echo the local rx pipeline:
```bash
ros2 topic echo /radio_rx
```
*Note: the node will not publish to the topic unless a terminal newline character `\n` is detected over the serial connection.*

## Additional Information
- **Baud Rate:** Defaults to `57600`, which is the standard for most telemetry radios. You can override this using the `baud_rate` ROS parameter:
  ```bash
  ros2 run ugv_radio_bridge radio_bridge_node --ros-args -p serial_port:=/dev/cu... -p baud_rate:=115200
  ```
- **Data Formatting:** The string parsing inherently expects new lines line-terminating characters (`\n`). If trailing carriage returns (`\r`) exist from the transmitting computer, they are stripped automatically prior to being sent via standard ROS2 logs.
- **Non-blocking Flow:** There is no hardware flow control overhead implemented here (RTS/CTS options are actively removed in code). It's uniquely built for lightweight telemetry streams.
