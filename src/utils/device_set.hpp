#ifndef ANTESTL_BACKEND_DEVICE_SET_HPP
#define ANTESTL_BACKEND_DEVICE_SET_HPP

#include "visa_device.hpp"

class DeviceSet {
    VisaDevice vna;
    VisaDevice external_gen;
    VisaDevice rbd;

public:
    DeviceSet() = default;


};

#endif //ANTESTL_BACKEND_DEVICE_SET_HPP
