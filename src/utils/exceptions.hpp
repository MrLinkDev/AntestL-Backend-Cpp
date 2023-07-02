#ifndef ANTESTL_BACKEND_EXCEPTIONS_HPP
#define ANTESTL_BACKEND_EXCEPTIONS_HPP

#define OPC_ERROR_MSG           "Unable to execute OPC"
#define OPC_ERROR_CODE          0xE0

#define READ_ERROR_MSG          "Can't read data from device"
#define READ_ERROR_CODE         0xE1

#define WRITE_ERROR_MSG         "Can't write data into device"
#define WRITE_ERROR_CODE        0xE2

#define DEVICE_ERROR_MSG        "Caught error from device"
#define DEVICE_ERROR_CODE       0xE3

#define NO_CONNECTION_MSG       "No connection with device"
#define NO_CONNECTION_CODE      0xEF

#define FREQ_OUT_OF_BOUND       0xF0
#define ANGLE_OUT_OF_BOUND      0xF1

class antestl_exception : public std::exception {
    std::string message;
    int code;

public:
    antestl_exception(std::string message, int code) {
        this->message = std::move(message);
        this->code = code;
    }

    std::string what() {
        return message;
    }

    int error_code() const {
        return code;
    }
};

#endif //ANTESTL_BACKEND_EXCEPTIONS_HPP
