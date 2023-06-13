#ifndef ANTESTL_BACKEND_VISA_DEVICE_HPP
#define ANTESTL_BACKEND_VISA_DEVICE_HPP

#include <cstring>
#include <windows.h>

#include "visa.h"
#include "../utils/logger.hpp"

#define BUFFER_SIZE             128

#define DEFAULT_TIMEOUT         10000
#define DEFAULT_VISA_TERM       '\n'

#define DEFAULT_OPC_SLEEP_TIME  250

#define SUCCESS                 0x00
#define FAILURE                 0x01

#define NO_ERRORS               0x02
#define ERRORS                  0x03

#define OPC_PASS                0x04
#define OPC_WAIT                0x05

#define CMD_IDN                 "*IDN?"
#define CMD_OPC                 "*OPC?"
#define CMD_ERR                 "SYSTEM:ERROR?"

#define NO_ERROR_STR            "+0,\"No error\""

#define OPC_PASS_STR_KEYSIGHT   "+1"
#define OPC_PASS_STR_PLANAR     "1"

struct visa_config {
    std::string address{};

    int timeout         = DEFAULT_TIMEOUT;
    char termination    = DEFAULT_VISA_TERM;
};

class VisaDevice {
    visa_config device_config{};

    ViSession resource_manager{};
    ViSession device{};

    ViStatus status{};
    ViPUInt32 ret_count{};

    int write(std::string command);
    std::string read();

    std::string query(std::string command);

protected:
    bool connected = false;

public:
    VisaDevice() = default;
    VisaDevice(const std::string device_address);
    VisaDevice(visa_config config);

    virtual void connect();

    bool is_connected();

    void clear();

    std::string idn();
    int opc();
    int err();

    void wait();

    std::string send(std::string command);
    std::string send_wait(std::string command);
    std::string send_err(std::string command);
    std::string send_wait_err(std::string command);

    template <typename... T>
    std::string send(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        try {
            data = send(command);
        } catch (int error_code) {
            throw error_code;
        }

        return data;
    }

    template <typename... T>
    std::string send_wait(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        try {
            data = send_wait(command);
        } catch (int error_code) {
            throw error_code;
        }

        return data;
    }

    template <typename... T>
    std::string send_err(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        try {
            data = send_err(command);
        } catch (int error_code) {
            throw error_code;
        }

        return data;
    }

    template <typename... T>
    std::string send_wait_err(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        try {
            data = send_wait_err(command);
        } catch (int error_code) {
            throw error_code;
        }

        return data;
    }
};

typedef VisaDevice visa_device;

#endif //ANTESTL_BACKEND_VISA_DEVICE_HPP
