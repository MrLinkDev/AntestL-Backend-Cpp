#ifndef ANTESTL_BACKEND_VISA_DEVICE_HPP
#define ANTESTL_BACKEND_VISA_DEVICE_HPP

#include "visa.h"
#include "logger.hpp"
#include <string>
#include <cstring>
#include <unistd.h>
#include <windows.h>

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

#define NO_ERROR_STR            "+0,\"No error\""

#define OPC_PASS_STR_KEYSIGHT   "+1"
#define OPC_PASS_STR_PLANAR     "1"

using namespace std;

struct visa_config {
    std::string address{};
    int timeout = DEFAULT_TIMEOUT;
    char term_char = DEFAULT_VISA_TERM;
};

class VisaDevice {

private:
    ViSession resource_manager{}, device{};
    ViStatus status{};

    ViPUInt32 ret_count{};
    visa_config device_config{};

protected:
    bool exist = false;

    virtual void setup_device() {
        status = viOpenDefaultRM(&resource_manager);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "Can't open resource manager");

            exist = false;
            return;
        } else {
            logger::log(LEVEL_DEBUG, "Opened default resource manager");
        }

        logger::log(LEVEL_INFO, "Connecting to device with address ", device_config.address.c_str(), NULL);

        status = viOpen(
                resource_manager,
                device_config.address.c_str(),
                VI_NULL,
                VI_NULL,
                &device);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "No connection to device...");

            exist = false;
            return;
        } else {
            logger::log(LEVEL_INFO, "Connected");
        }

        status = viSetAttribute(device, VI_ATTR_TMO_VALUE, DEFAULT_TIMEOUT);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "Can't set timeout");

            exist = false;
            return;
        } else {
            logger::log(LEVEL_DEBUG, "Timeout set to ", to_string(device_config.timeout).c_str(), NULL);
        }

        status = viSetAttribute(device, VI_ATTR_TERMCHAR, DEFAULT_VISA_TERM);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "Can't set termination character");

            exist = false;
            return;
        } else {
            logger::log(LEVEL_DEBUG, "Termination character set to (code) ", to_string((int) device_config.term_char).c_str(), NULL);
        }

        exist = true;
    }

public:
    bool using_ext_gen = false;

    VisaDevice() = default;

    VisaDevice(const string device_address) {
        device_config.address = device_address;

        setup_device();
    }

    VisaDevice(visa_config config) {
        device_config = config;

        setup_device();
    }

    bool is_exist() {
        return exist;
    }

    void clear() {
        viClear(device);
    }

    int write(string command) {
        logger::log(LEVEL_DEBUG, "WRITE: ", command.c_str(), NULL);
        command += device_config.term_char;

        ViConstBuf command_buffer = reinterpret_cast<ViConstBuf>(command.c_str());
        status = viWrite(device, command_buffer, command.length(), ret_count);

        if (status < VI_SUCCESS) {
            return FAILURE;
        }

        return SUCCESS;
    }

    int read(string &data) {
        ViChar buffer[BUFFER_SIZE];

        while (true) {
            status = viRead(device, reinterpret_cast<ViPBuf>(buffer), BUFFER_SIZE, ret_count);

            if (status < VI_SUCCESS) {
                return FAILURE;
            }

            for (char b : buffer) {
                if (b == device_config.term_char) {
                    logger::log(LEVEL_DEBUG, "READ: ", data.c_str(), NULL);
                    return SUCCESS;
                }

                data += b;
            }

            memset(buffer, 0, sizeof(buffer));
        }
    }

    int query(string command, string *data) {
        if (write(command) == FAILURE) {
            return FAILURE;
        }

        if (read(*data) == FAILURE) {
            return FAILURE;
        }

        return SUCCESS;
    }

    int check_error() {
        string error_info{};
        if (this->query("SYSTEM:ERROR?", &error_info) == SUCCESS) {
            if (error_info == NO_ERROR_STR) {
                return NO_ERRORS;
            } else {
                logger::log(LEVEL_ERROR, error_info);
                return ERRORS;
            }
        }

        return FAILURE;
    }

    int opc() {
        string opc_info{};
        if (this->query("*OPC?", &opc_info) == SUCCESS) {
            if (opc_info == OPC_PASS_STR_KEYSIGHT || opc_info == OPC_PASS_STR_PLANAR) {
                return OPC_PASS;
            } else {
                return OPC_WAIT;
            }
        }

        return FAILURE;
    }

    void wait() {
        while(opc() != OPC_PASS) {
            Sleep(DEFAULT_OPC_SLEEP_TIME);
        }
    }

    int idn() {
        string device_info;

        if (query(CMD_IDN, &device_info) == FAILURE) {
            return FAILURE;
        }

        logger::log(LEVEL_INFO, "Device info: ", device_info.c_str(), NULL);
        return SUCCESS;
    }

    void send(string command, string *data = nullptr) {
        if (command[command.length() - 1] == '?') {
            if (data == nullptr) {
                logger::log(LEVEL_ERROR, "Got query, but no buffer for reading...", NULL);
                throw FAILURE;
            }

            if (query(command, data) == FAILURE) {
                throw FAILURE;
            }

        } else {
            if (write(command) == FAILURE) {
                throw FAILURE;
            }
        }
    }

    void send_wait(string command, string *data = nullptr) {
        try {
            send(command, data);
        } catch (int code) {
            if (code == FAILURE) {
                throw FAILURE;
            }
        }

        wait();
    }

    void send_err(string command, string *data = nullptr) {
        try {
            send(command, data);
        } catch (int code) {
            if (code == FAILURE) {
                throw FAILURE;
            }
        }

        int error_status =  check_error();

        if (error_status == FAILURE || error_status == ERRORS) {
            throw FAILURE;
        }
    }

    void send_wait_err(string command, string *data = nullptr) {
        try {
            send_wait(command, data);
        } catch (int code) {
            if (code == FAILURE) {
                throw FAILURE;
            }
        }

        int error_status =  check_error();

        if (error_status == FAILURE || error_status == ERRORS) {
            throw FAILURE;
        }
    }
};

#endif //ANTESTL_BACKEND_VISA_DEVICE_HPP
