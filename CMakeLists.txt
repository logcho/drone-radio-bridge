cmake_minimum_required(VERSION 3.16)
project(Bridge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(main
    main.cpp
    src/serial_port.cpp
)
