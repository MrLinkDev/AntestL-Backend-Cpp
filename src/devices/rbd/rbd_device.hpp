#ifndef ANTESTL_BACKEND_RBD_DEVICE_HPP
#define ANTESTL_BACKEND_RBD_DEVICE_HPP

#include "../visa_device.hpp"
#include <vector>

#define ANGLE_MOVE_OK       0xF2
#define ANGLE_MOVE_BOUND    0xF3

#define ADDRESS_DELIMITER   ';'

class RbdDevice {

protected:
    std::vector<float> start_angle{};
    std::vector<float> stop_angle{};

    std::vector<float> angle_step{};
    std::vector<float> current_angle{};

    std::vector<int> points{};
    std::vector<int> current_point{};

    void init_params(int axis_count = 0) {
        for (int i = 0; i < axis_count; ++i) {
            this->start_angle.push_back(0);
            this->stop_angle.push_back(0);

            this->angle_step.push_back(0);
            this->current_angle.push_back(0);

            this->points.push_back(0);
            this->current_point.push_back(0);
        }
    };

public:
    RbdDevice() = default;

    virtual bool is_connected() {return false;};

    virtual void move(float pos, int axis_num) {};
    virtual void stop() {};

    virtual void set_angle(float angle, int axis_num) {};
    virtual void set_angle_range(float start_angle, float stop_angle, int points, int axis_num) {};

    virtual int next_angle(int axis_num) {return ANGLE_MOVE_BOUND;};
    virtual int prev_angle(int axis_num) {return ANGLE_MOVE_BOUND;};

    virtual void move_to_start_angle(int axis_num) {};
    virtual void move_to_stop_angle(int axis_num) {};

    virtual float get_pos(int axis_num) {return 0.0f;};

    virtual int get_axes_count() {return 0;}
};

#endif //ANTESTL_BACKEND_RBD_DEVICE_HPP
