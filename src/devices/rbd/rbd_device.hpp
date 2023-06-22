#ifndef ANTESTL_BACKEND_RBD_DEVICE_HPP
#define ANTESTL_BACKEND_RBD_DEVICE_HPP

#include "../visa_device.hpp"

class RbdDevice {

protected:
    std::vector<float> start_angle{};
    std::vector<float> stop_angle{};

    std::vector<float> angle_step{};
    std::vector<float> current_angle{};

    std::vector<int> points{};
    std::vector<int> current_point{};

    virtual void init_params(size_t axis_count = 1) {};

public:
    RbdDevice() = default;

    virtual bool is_connected() {return false;};

    virtual void move(float pos, int axis_num = 0) {};
    virtual void stop() {};

    virtual void set_angle(float angle, int axis_num = 0) {};
    virtual void set_angle_range(float start_angle, float stop_angle, int points, int axis_num = 0) {};

    virtual void next_angle(int axis_num = 0) {};
    virtual void prev_angle(int axis_num = 0) {};

    virtual void move_to_start_angle(int axis_num = 0) {};
    virtual void move_to_stop_angle(int axis_num = 0) {};

    virtual float get_pos(int axis_num = 0) {return 0.0f;};
};

#endif //ANTESTL_BACKEND_RBD_DEVICE_HPP
