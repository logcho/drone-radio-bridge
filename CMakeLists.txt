cmake_minimum_required(VERSION 3.8)
project(ugv_radio_bridge)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(radio_bridge_node src/radio_bridge_node.cpp)
target_include_directories(radio_bridge_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(radio_bridge_node
  rclcpp
  std_msgs
)

install(TARGETS radio_bridge_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()
