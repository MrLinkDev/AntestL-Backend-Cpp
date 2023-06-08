#ifndef ANTESTL_BACKEND_RBD_DEVICE_HPP
#define ANTESTL_BACKEND_RBD_DEVICE_HPP

#include "../../utils/visa_device.hpp"

class RbdDevice : public VisaDevice {
    VisaDevice *axes;
    int axis_count = 0;

public:
    RbdDevice() = default;

    RbdDevice(const string device_address) : VisaDevice(device_address) {}

    virtual void axis_info() {
        logger::log(LEVEL_DEBUG, "Called \"axis_info\" function");
    }
};

#endif //ANTESTL_BACKEND_RBD_DEVICE_HPP
