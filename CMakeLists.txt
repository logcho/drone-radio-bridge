cmake_minimum_required(VERSION 3.8)
project(ugv_radio_bridge)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(radio_bridge_node src/radio_bridge_node.cpp)
target_include_directories(radio_bridge_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(radio_bridge_node
  rclcpp
  std_msgs
)

add_executable(uav_command_parser_node src/uav_command_parser.cpp)
ament_target_dependencies(uav_command_parser_node rclcpp std_msgs geometry_msgs)

add_executable(uav_test_node src/uav_test_node.cpp)
ament_target_dependencies(uav_test_node rclcpp std_msgs geometry_msgs)

add_executable(uav_example_sender_node src/uav_example_sender.cpp)
ament_target_dependencies(uav_example_sender_node rclcpp std_msgs geometry_msgs)

add_executable(uav_interactive_sender_node src/uav_interactive_sender.cpp)
ament_target_dependencies(uav_interactive_sender_node rclcpp std_msgs geometry_msgs)

install(TARGETS 
  radio_bridge_node
  uav_command_parser_node
  uav_test_node
  uav_example_sender_node
  uav_interactive_sender_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()
