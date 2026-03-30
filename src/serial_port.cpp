#include "serial_port.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <iostream>

int SerialPort::baudToConstant(int baudrate) {
    switch(baudrate) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
#ifdef B460800
        case 460800: return B460800;
#endif
#ifdef B500000
        case 500000: return B500000;
#endif
#ifdef B576000
        case 576000: return B576000;
#endif
#ifdef B921600
        case 921600: return B921600;
#endif
#ifdef B1000000
        case 1000000: return B1000000;
#endif
#ifdef B1152000
        case 1152000: return B1152000;
#endif
#ifdef B1500000
        case 1500000: return B1500000;
#endif
#ifdef B2000000
        case 2000000: return B2000000;
#endif
#ifdef B2500000
        case 2500000: return B2500000;
#endif
#ifdef B3000000
        case 3000000: return B3000000;
#endif
#ifdef B3500000
        case 3500000: return B3500000;
#endif
#ifdef B4000000
        case 4000000: return B4000000;
#endif
        default: return -1;
    }
}

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

    int posix_baud = baudToConstant(baudrate_);
    if (posix_baud == -1) {
        std::cerr << "Unsupported baud rate: " << baudrate_ << std::endl;
        close(fd_);
        fd_ = -1;
        return false;
    }

    cfsetispeed(&tty, posix_baud);
    cfsetospeed(&tty, posix_baud);

    tty.c_cflag |= (CLOCAL | CREAD); // Ignore modem controls, enable reading
    tty.c_cc[VMIN]  = 0;            // Non-blocking
    tty.c_cc[VTIME] = 0;            // Time to wait (0 = don't wait)

    // apply settings
    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr" << std::endl;
        close(fd_);
        fd_ = -1;
        return false;
    }

    tcflush(fd_, TCIOFLUSH); // Flush stale data

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