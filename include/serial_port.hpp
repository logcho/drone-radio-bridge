#pragma once
#include <string>
#include <cstdint>

class SerialPort {
    public:
        SerialPort(const std::string& device, int baudrate);
        ~SerialPort();

        // open the port; returns true if successful
        bool openPort();

        // read bytes into a buffer; returns number of bytes read
        int readBytes(uint8_t* buf, size_t size);

        // write bytes from a buffer; return number of bytes written
        int writeBytes(const uint8_t* buf, size_t size);

    private:
        std::string device_;
        int baudrate_;
        int fd_;
};