#ifndef ANTESTL_BACKEND_DEMO_RDB_HPP
#define ANTESTL_BACKEND_DEMO_RDB_HPP

#include <thread>
#include "rbd_device.hpp"
#include "../../utils/exceptions.hpp"

using namespace std::chrono_literals;

class DemoRbd : public RbdDevice {
    int axes_count = 0;
    std::vector<float> axes_pos;

    int status(int axis_num);
    int status(VisaDevice *axis);

    void init_params(size_t axis_count) {
        for (size_t i = 0; i < axes_count; ++i) {
            axes_pos.push_back(0.0f);

            start_angle.push_back(0.0f);
            stop_angle.push_back(0.0f);

            angle_step.push_back(0.0f);
            current_angle.push_back(0.0f);

            points.push_back(0);
            current_point.push_back(0);
        }
    };

public:
    DemoRbd() = default;
    DemoRbd(const std::string& device_addresses) {
        axes_count = string_utils::count(device_addresses, ';') + 1;

        init_params(0);
    }

    bool is_connected() {
        return true;
    };

    void move(float pos, int axis_num) {
        axes_pos[axis_num] = pos;
    };

    void stop() {};

    void set_angle(float angle, int axis_num = 0) {
        this->start_angle[axis_num] = angle;
        this->stop_angle[axis_num] = angle;

        this->current_angle[axis_num] = angle;

        this->points[axis_num] = 1;
        this->current_point[axis_num] = 0;

        move(angle, axis_num);
    }

    void set_angle_range(float start_angle, float stop_angle, int points, int axis_num = 0) {
        this->start_angle[axis_num] = start_angle;
        this->stop_angle[axis_num] = stop_angle;

        this->points[axis_num] = points;

        angle_step[axis_num] =
                this->points[axis_num] <= 1 ?
                0 : (this->stop_angle[axis_num] - this->start_angle[axis_num]) / (this->points[axis_num] - 1);

        this->current_point[axis_num] = 0;
        this->current_angle[axis_num] = start_angle;
    };

    void next_angle(int axis_num = 0) {
        if (current_point[axis_num] == points[axis_num] - 1) {
            throw ANGLE_OUT_OF_BOUND;
        }

        current_angle[axis_num] += angle_step[axis_num];
        ++current_point[axis_num];

        move(current_angle[axis_num], axis_num);
    };
    void prev_angle(int axis_num = 0) {
        if (current_point[axis_num] == 0) {
            throw ANGLE_OUT_OF_BOUND;
        }

        current_angle[axis_num] -= angle_step[axis_num];
        --current_point[axis_num];

        move(current_angle[axis_num], axis_num);
    };

    void move_to_start_angle(int axis_num = 0) {
        current_angle[axis_num] = start_angle[axis_num];
        current_point[axis_num] = 0;

        move(current_angle[axis_num], axis_num);
    };
    void move_to_stop_angle(int axis_num = 0) {
        current_angle[axis_num] = stop_angle[axis_num];
        current_point[axis_num] = points[axis_num] - 1;

        move(current_angle[axis_num], axis_num);
    };

    float get_pos(int axis_num) {
        return axes_pos[axis_num];
    };

    int get_axes_count() {
        return axes_count;
    };
};

#endif //ANTESTL_BACKEND_DEMO_RDB_HPP
