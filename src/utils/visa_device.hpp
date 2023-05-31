#ifndef ANTESTL_BACKEND_VISA_DEVICE_HPP
#define ANTESTL_BACKEND_VISA_DEVICE_HPP

#include "visa.h"
#include "logger.hpp"
#include <string>
#include <cstring>

#define BUFFER_SIZE             128

#define DEFAULT_TIMEOUT         10000
#define DEFAULT_TERMINATION     '\n'

#define SUCCESS                 0x00
#define FAILURE                 0x01

#define CMD_IDN                 "*IDN?"

using namespace std;

struct visa_config {
    std::string address{};
    int timeout = DEFAULT_TIMEOUT;
    char term_char = DEFAULT_TERMINATION;
};

class VisaDevice {
    ViSession resource_manager, device;
    ViStatus status;

    ViPUInt32 ret_count;
    visa_config device_config{};

    bool exist = false;

public:
    VisaDevice() = default;

    VisaDevice(const string device_address) {
        device_config.address = device_address;

        status = viOpenDefaultRM(&resource_manager);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "Can't open resource manager");

            exist = false;
            return;
        } else {
            logger::log(LEVEL_DEBUG, "Opened default resource manager");
        }

        logger::log(LEVEL_INFO, "Connecting to device with address", device_config.address.c_str(), NULL);

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
            logger::log(LEVEL_INFO, "Connected to device");
        }

        status = viSetAttribute(device, VI_ATTR_TMO_VALUE, DEFAULT_TIMEOUT);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "Can't set timeout");

            exist = false;
            return;
        } else {
            logger::log(LEVEL_DEBUG, "Timeout set to", to_string(device_config.timeout).c_str(), NULL);
        }

        status = viSetAttribute(device, VI_ATTR_TERMCHAR, DEFAULT_TERMINATION);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "Can't set termination character");

            exist = false;
            return;
        } else {
            logger::log(LEVEL_DEBUG, "Termination character set to (code)", to_string((int) device_config.term_char).c_str(), NULL);
        }

        exist = true;

        string device_info;
        this->idn(device_info);

        logger::log(LEVEL_INFO, "Device info:", device_info.c_str(), NULL);
    }

    VisaDevice(visa_config config) {

    }

    int write(string command) {
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

            for (int pos = 0; pos < BUFFER_SIZE; ++pos) {
                if (buffer[pos] == device_config.term_char) {
                    return SUCCESS;
                }

                data += buffer[pos];
            }

            memset(buffer, 0, sizeof(buffer));
        }
    }

    int query(string command, string &data) {
        int status;

        status = write(command);
        if (status == FAILURE) {
            return FAILURE;
        }

        status = read(data);
        if (status == FAILURE) {
            return FAILURE;
        }

        return SUCCESS;
    }
    
    int idn(string &device_info) {
        return query(CMD_IDN, device_info);
    }
};

#endif //ANTESTL_BACKEND_VISA_DEVICE_HPP
