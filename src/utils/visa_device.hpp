#ifndef ANTESTL_BACKEND_VISA_DEVICE_HPP
#define ANTESTL_BACKEND_VISA_DEVICE_HPP

#include "visa.h"
#include "logger.hpp"
#include <string>
#include <cstring>

#define READ_BUFFER_SIZE        128

#define DEFAULT_TIMEOUT         10000
#define DEFAULT_TERMINATION     '\n'

#define WRITE_SUCCESS           0x00
#define WRITE_FAILURE           0x01

#define READ_SUCCESS            0x02
#define READ_FAILURE            0x03

#define QUERY_SUCCESS           0x04
#define QUERY_FAILURE           0x05

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

        stringstream message;
        message << "Connecting to device with address '" << device_config.address << "'";
        logger::log(LEVEL_INFO, message.str());

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
            stringstream message;
            message << "Timeout set to " << device_config.timeout;

            logger::log(LEVEL_INFO, message.str());
        }

        status = viSetAttribute(device, VI_ATTR_TERMCHAR, DEFAULT_TERMINATION);

        if (status < VI_SUCCESS) {
            logger::log(LEVEL_ERROR, "Can't set termination character");

            exist = false;
            return;
        } else {
            stringstream message;
            message << "Termination character set to (code) " << (int) device_config.term_char;

            logger::log(LEVEL_INFO, message.str());
        }

        exist = true;


    }

    VisaDevice(visa_config config) {

    }

    int write(string command) {
        command += device_config.term_char;

        ViConstBuf command_buffer = reinterpret_cast<ViConstBuf>(command.c_str());
        status = viWrite(device, command_buffer, command.length(), ret_count);

        if (status < VI_SUCCESS) {
            return WRITE_FAILURE;
        }

        return WRITE_SUCCESS;
    }

    int read(string &data) {
        ViChar buffer[READ_BUFFER_SIZE];

        while (true) {
            status = viRead(device, reinterpret_cast<ViPBuf>(buffer), READ_BUFFER_SIZE, ret_count);

            if (status < VI_SUCCESS) {
                return READ_FAILURE;
            }

            for (int pos = 0; pos < READ_BUFFER_SIZE; ++pos) {
                if (buffer[pos] == device_config.term_char) {
                    return READ_SUCCESS;
                }

                data += buffer[pos];
            }

            memset(buffer, 0, sizeof(buffer));
        }
    }

    int query(string command, string &data) {
        if (write(command) == WRITE_FAILURE) {
            return QUERY_FAILURE;
        }

        if (read(data) == READ_FAILURE) {
            return QUERY_FAILURE;
        }

        return QUERY_SUCCESS;
    }
};

#endif //ANTESTL_BACKEND_VISA_DEVICE_HPP
