cmake_minimum_required(VERSION 3.16)
project(Bridge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_ROS2 "Build ROS2 node" OFF)

include_directories(include)

add_library(crc16
    src/crc16.cpp
)

add_executable(main
    main.cpp
)

add_executable(test_serial_port
    test/test_serial_port.cpp
    src/serial_port.cpp
)

add_executable(test_packet_parser
    test/test_packet_parser.cpp
    src/packet_parser.cpp
    src/packet_encoder.cpp
)

target_link_libraries(test_packet_parser
    crc16
)

add_executable(test_packet_encoder
    test/test_packet_encoder.cpp
    src/packet_encoder.cpp
)

target_link_libraries(test_packet_encoder
    crc16
)

add_executable(test_crc_roundtrip
    test/test_crc_roundtrip.cpp
    src/packet_parser.cpp
    src/packet_encoder.cpp
)

target_link_libraries(test_crc_roundtrip
    crc16
)

add_executable(test_serial_roundtrip
    test/test_serial_roundtrip.cpp
    src/serial_port.cpp
    src/packet_parser.cpp
    src/packet_encoder.cpp
    src/crc16.cpp
)

target_include_directories(test_serial_roundtrip PUBLIC include)

add_executable(test_local_integration
    test/test_local_integration.cpp
    src/serial_port.cpp
    src/packet_parser.cpp
    src/packet_encoder.cpp
    src/crc16.cpp
)

if(BUILD_ROS2)
  message(STATUS "Building ROS2 radio bridge node")

  find_package(ament_cmake REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(geometry_msgs REQUIRED)

  add_executable(radio_bridge_node
    src/radio_bridge_node_ros2.cpp
    src/serial_port.cpp
    src/packet_parser.cpp
    src/packet_encoder.cpp
    src/crc16.cpp
  )

  target_compile_definitions(radio_bridge_node PRIVATE BUILD_ROS2)

  ament_target_dependencies(radio_bridge_node
    rclcpp
    geometry_msgs
  )

  install(TARGETS radio_bridge_node
    DESTINATION lib/${PROJECT_NAME}
  )

  ament_package()
endif()
