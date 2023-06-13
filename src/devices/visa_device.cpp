#include "visa_device.hpp"
#include "../utils/exceptions.hpp"

int VisaDevice::write(std::string command) {
    logger::log(LEVEL_DEBUG, "WRITE: {}", command);
    command += device_config.termination;

    ViConstBuf command_buffer = reinterpret_cast<ViConstBuf>(command.c_str());
    status = viWrite(device, command_buffer, command.length(), ret_count);

    if (status < VI_SUCCESS) {
        return FAILURE;
    }

    return SUCCESS;
}

std::string VisaDevice::read() {
    std::string data{};
    ViChar buffer[BUFFER_SIZE];

    while (true) {
        status = viRead(device, reinterpret_cast<ViPBuf>(buffer), BUFFER_SIZE, ret_count);

        if (status < VI_SUCCESS) {
            return std::string{};
        }

        for (char b : buffer) {
            if (b == device_config.termination) {
                logger::log(LEVEL_DEBUG, "READ: {}", data);
                return data;
            }

            data.push_back(b);
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

std::string VisaDevice::query(std::string command) {
    std::string data{};

    if (write(command) == FAILURE) {
        return std::string{};
    }

    data = read();
    return data;
}

VisaDevice::VisaDevice(const std::string device_address) {
    device_config.address = device_address;
}

VisaDevice::VisaDevice(visa_config config) {
    device_config = config;
}

void VisaDevice::connect() {
    status = viOpenDefaultRM(&resource_manager);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't open resource manager");

        connected = false;
        return;
    } else {
        logger::log(LEVEL_DEBUG, "Opened default resource manager");
    }

    logger::log(LEVEL_INFO, "Connecting to device with address {}", device_config.address);

    status = viOpen(
            resource_manager,
            device_config.address.c_str(),
            VI_NULL,
            VI_NULL,
            &device);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "No connection to device...");

        connected = false;
        return;
    } else {
        logger::log(LEVEL_INFO, "Connected");
    }

    status = viSetAttribute(device, VI_ATTR_TMO_VALUE, DEFAULT_TIMEOUT);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't set timeout");

        connected = false;
        return;
    } else {
        logger::log(LEVEL_DEBUG, "Timeout set to {}", device_config.timeout);
    }

    status = viSetAttribute(device, VI_ATTR_TERMCHAR, DEFAULT_VISA_TERM);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't set termination character");

        connected = false;
        return;
    } else {
        logger::log(LEVEL_DEBUG, "Termination character set to (code) {}", ((int) device_config.termination));
    }

    connected = true;
}

bool VisaDevice::is_connected() {
    return connected;
}

void VisaDevice::clear() {
    viClear(device);
}

std::string VisaDevice::idn() {
    std::string device_info;

    device_info = query(CMD_IDN);

    if (device_info.empty()) {
        logger::log(LEVEL_WARN, "No IDN data from device");
    } else {
        logger::log(LEVEL_INFO, "Device info: {}", device_info);
    }

    return device_info;
}

int VisaDevice::opc() {
    std::string opc_info{};

    opc_info = query(CMD_OPC);

    if (opc_info.empty()) {
        logger::log(LEVEL_WARN, "No OPC data from device");
    } else {
        if (opc_info == OPC_PASS_STR_KEYSIGHT || opc_info == OPC_PASS_STR_PLANAR) {
            return OPC_PASS;
        } else {
            return OPC_WAIT;
        }
    }

    return FAILURE;
}

int VisaDevice::err() {
    std::string error_info{};

    error_info = query(CMD_ERR);

    if (error_info.empty()) {
        logger::log(LEVEL_WARN, "No error data from device");
    } else {
        if (error_info == NO_ERROR_STR) {
            return NO_ERRORS;
        } else {
            logger::log(LEVEL_ERROR, error_info);
            return ERRORS;
        }
    }

    return FAILURE;
}

void VisaDevice::wait() {
    int opc_status;

    do {
        opc_status = opc();

        if (opc_status == FAILURE) {
            logger::log(LEVEL_ERROR, "Unable to execute OPC");
            throw GOT_ERROR_FROM_OPC;
        }

    } while (opc_status != OPC_PASS);
}

std::string VisaDevice::send(std::string command) {
    std::string data{};

    if (command[command.length() - 1] == '?') {
        data = query(command);

        if (data.empty()) {
            logger::log(LEVEL_ERROR, "No data from device");
            throw NO_DATA_FROM_DEVICE;
        }
    } else {
        if (write(command) == FAILURE) {
            logger::log(LEVEL_ERROR, "Unable to send data");
            throw GOT_ERROR_FROM_WRITE;
        }
    }

    return data;
}

std::string VisaDevice::send_wait(std::string command) {
    std::string data{};

    try {
        data = send(command);
        wait();
    } catch (int error_code) {
        throw error_code;
    }

    return data;
}

std::string VisaDevice::send_err(std::string command) {
    std::string data{};

    try {
        data = send(command);
    } catch (int error_code) {
        throw error_code;
    }

    int error_status = err();

    if (error_status == FAILURE || error_status == ERRORS) {
        throw GOT_ERROR_FROM_DEVICE;
    }

    return data;
}

std::string VisaDevice::send_wait_err(std::string command) {
    std::string data{};

    try {
        data = send(command);
        wait();
    } catch (int error_code) {
        throw error_code;
    }

    int error_status = err();

    if (error_status == FAILURE || error_status == ERRORS) {
        throw GOT_ERROR_FROM_DEVICE;
    }

    return data;
}
