#include "visa_device.hpp"
#include "../utils/exceptions.hpp"

int VisaDevice::write(std::string command) {
    logger::log(LEVEL_TRACE, "WRITE: {}", command);
    command += device_config.termination;

    auto command_buffer = reinterpret_cast<ViConstBuf>(command.c_str());
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
                logger::log(LEVEL_TRACE, "READ: {}", data);
                return data;
            }

            data.push_back(b);
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

std::string VisaDevice::query(std::string command) {
    std::string data{};

    if (write(std::move(command)) == FAILURE) {
        return std::string{};
    }

    data = read();
    return data;
}

VisaDevice::VisaDevice(std::string device_address) {
    device_config.address = std::move(device_address);
}

VisaDevice::~VisaDevice() {
    if (connected) {
        clear();
        viClose(device);

        connected = false;
    }
}

void VisaDevice::connect() {
    status = viOpenDefaultRM(&resource_manager);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't open resource manager");

        connected = false;
        return;
    }

    logger::log(LEVEL_TRACE, "Connecting to device with address {}", device_config.address);

    status = viOpen(
            resource_manager,
            device_config.address.c_str(),
            VI_NULL,
            VI_NULL,
            &device);

    if (status < VI_SUCCESS) {
        connected = false;
        return;
    } else {
        logger::log(LEVEL_DEBUG, "Connected to device with address {}", device_config.address);
    }

    status = viSetAttribute(device, VI_ATTR_TMO_VALUE, DEFAULT_TIMEOUT);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't set timeout");

        connected = false;
        return;
    }

    status = viSetAttribute(device, VI_ATTR_TERMCHAR, DEFAULT_VISA_TERM);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't set termination character");

        connected = false;
        return;
    }

    connected = true;
}

bool VisaDevice::is_connected() const {
    return connected;
}

void VisaDevice::clear() const {
    viClear(device);
}

std::string VisaDevice::idn() {
    std::string device_info;

    device_info = query(CMD_IDN);

    if (device_info.empty()) {
        logger::log(LEVEL_WARN, "No IDN data from device");
    } else {
        logger::log(LEVEL_DEBUG, "Device info: {}", device_info);
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
    std::string error_info = query(CMD_ERR);

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
            logger::log(LEVEL_ERROR, OPC_ERROR_MSG);
            throw antestl_exception(OPC_ERROR_MSG, OPC_ERROR_CODE);
        }

    } while (opc_status != OPC_PASS);
}

std::string VisaDevice::send(std::string command, bool read_data) {
    std::string data{};

    if (command[command.length() - 1] == '?') {
        data = query(command);

        if (data.empty()) {
            logger::log(LEVEL_ERROR, READ_ERROR_MSG);
            throw antestl_exception(READ_ERROR_MSG, READ_ERROR_CODE);
        }
    } else {
        if (write(command) == FAILURE) {
            logger::log(LEVEL_ERROR, WRITE_ERROR_MSG);
            throw antestl_exception(WRITE_ERROR_MSG, WRITE_ERROR_CODE);
        }

        if (read_data) {
            data = read();

            if (data.empty()) {
                logger::log(LEVEL_ERROR, READ_ERROR_MSG);
                throw antestl_exception(READ_ERROR_MSG, READ_ERROR_CODE);
            }
        }
    }

    return data;
}

std::string VisaDevice::send_wait(std::string command) {
    std::string data{};

    data = send(std::move(command));
    wait();

    return data;
}

std::string VisaDevice::send_err(std::string command) {
    std::string data{};

    data = send(std::move(command));

    if (err() == ERRORS) {
        logger::log(LEVEL_ERROR, DEVICE_ERROR_MSG);
        throw antestl_exception(DEVICE_ERROR_MSG, DEVICE_ERROR_CODE);
    }

    return data;
}

std::string VisaDevice::send_wait_err(std::string command) {
    std::string data{};

    send_wait(std::move(command));

    if (err() == ERRORS) {
        logger::log(LEVEL_ERROR, DEVICE_ERROR_MSG);
        throw antestl_exception(DEVICE_ERROR_MSG, DEVICE_ERROR_CODE);
    }

    return data;
}
