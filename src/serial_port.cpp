#include "serial_port.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <iostream>

SerialPort::SerialPort(const std::string& device, int baudrate) : device_(device), baudrate_(baudrate), fd_(-1) {}

SerialPort::~SerialPort() {
    if(fd_ != -1){
        close(fd_);
    }
}

bool SerialPort::openPort(){
    fd_ = open(device_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(fd_ < 0){
        std::cerr << "Failed to open serial port: " << device_ << std::endl;
        return false;
    }

    termios tty{};

    if(tcgetattr(fd_, &tty) != 0){
        std::cerr << "Error from tcgetattr" << std::endl;
        return false;
    }

    cfmakeraw(&tty);

    cfsetispeed(&tty, baudrate_);
    cfsetospeed(&tty, baudrate_);


    // apply settings
    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr" << std::endl;
        return false;
    }

    return true;
}

int SerialPort::readBytes(uint8_t* buf, size_t size){
    if(fd_ < 0) return -1;
    return read(fd_, buf, size);
}   

int SerialPort::writeBytes(const uint8_t* buf, size_t size){
    if(fd_ < 0) return -1;
    return write(fd_, buf, size);
}