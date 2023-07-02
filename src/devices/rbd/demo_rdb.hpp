#ifndef ANTESTL_BACKEND_DEMO_RDB_HPP
#define ANTESTL_BACKEND_DEMO_RDB_HPP

#include <thread>
#include "rbd_device.hpp"
#include "../../utils/exceptions.hpp"
#include "../../utils/string_utils.hpp"

class DemoRbd : public RbdDevice {
protected:
    int axis_count = 0;
    std::vector<float> axis_pos{};

public:
    DemoRbd(const std::string &device_addresses):RbdDevice() {
        axis_count = string_utils::count(device_addresses, ADDRESS_DELIMITER) + 1;

        for (int i = 0; i < axis_count; ++i) {
            axis_pos.push_back(0);
        }

        init_params(axis_count);
    }

    bool is_connected() override {
        return true;
    };

    void move(float pos, int axis_num) override {
        axis_pos[axis_num] = pos;
    };

    void set_angle(float angle, int axis_num) override {
        this->start_angle[axis_num] = angle;
        this->stop_angle[axis_num] = angle;

        this->current_angle[axis_num] = angle;

        this->points[axis_num] = 1;
        this->current_point[axis_num] = 0;

        move(angle, axis_num);
    };

    void set_angle_range(float start_angle, float stop_angle, int points, int axis_num) override {
        this->start_angle[axis_num] = start_angle;
        this->stop_angle[axis_num] = stop_angle;
        this->points[axis_num] = points;

        angle_step[axis_num] =
                this->points[axis_num] <= 1 ?
                0 : (this->stop_angle[axis_num] - this->start_angle[axis_num]) / (this->points[axis_num] - 1);

        this->current_angle[axis_num] = start_angle;
        this->current_point[axis_num] = 0;
    };

    int next_angle(int axis_num) override {
        if (current_point[axis_num] == points[axis_num] - 1) {
            return ANGLE_MOVE_BOUND;
        }

        current_angle[axis_num] += angle_step[axis_num];
        ++current_point[axis_num];

        move(current_angle[axis_num], axis_num);

        return ANGLE_MOVE_OK;
    };

    int prev_angle(int axis_num) override {
        if (current_point[axis_num] == 0) {
            return ANGLE_MOVE_BOUND;
        }

        current_angle[axis_num] -= angle_step[axis_num];
        --current_point[axis_num];

        return ANGLE_MOVE_OK;
    };

    void move_to_start_angle(int axis_num) override {
        current_angle[axis_num] = start_angle[axis_num];
        current_point[axis_num] = 0;

        move(current_angle[axis_num], axis_num);
    };

    void move_to_stop_angle(int axis_num) override {
        current_angle[axis_num] = stop_angle[axis_num];
        current_point[axis_num] = points[axis_num] - 1;

        move(current_angle[axis_num], axis_num);
    };

    float get_pos(int axis_num) override {
        return axis_pos[axis_num];
    };

    int get_axes_count() override {
        return axis_count;
    }
};

#endif //ANTESTL_BACKEND_DEMO_RDB_HPP
