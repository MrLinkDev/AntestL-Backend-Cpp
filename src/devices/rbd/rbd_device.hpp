#ifndef ANTESTL_BACKEND_RBD_DEVICE_HPP
#define ANTESTL_BACKEND_RBD_DEVICE_HPP

#include "../visa_device.hpp"

class RbdDevice {

protected:
    float start_angle;
    float stop_angle;

    float angle_step;

    int points;

public:
    RbdDevice() = default;

    virtual void move(float pos, int axis_num = 0) {};
    virtual void stop() {};

    virtual float get_pos(int axis_num = 0) {};
};

#endif //ANTESTL_BACKEND_RBD_DEVICE_HPP
