cmake_minimum_required(VERSION 3.16)
project(Bridge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

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
)