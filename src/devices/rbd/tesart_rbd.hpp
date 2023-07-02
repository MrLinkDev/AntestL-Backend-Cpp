#ifndef ANTESTL_BACKEND_TESART_RBD_HPP
#define ANTESTL_BACKEND_TESART_RBD_HPP

#include <thread>
#include "rbd_device.hpp"

#define TESART_RBD_SLEEP_TIME_INIT      10000ms
#define TESART_RBD_SLEEP_TIME_WAIT      10000ms

#define BIT_REF_SET                     0x00020000
#define BIT_IN_POS                      0x00080000
#define BIT_MOVE_BLOCK                  0x00010000

#define SCALE                           1000

using namespace std::chrono_literals;

class TesartRbd : public RbdDevice {
    std::vector<VisaDevice> axes;

    int velocity = 50;
    int acceleration = 3;

    int status(int axis_num);
    int status(VisaDevice *axis);

public:
    TesartRbd() = default;
    explicit TesartRbd(const std::string &device_addresses);

    bool is_connected() override;

    bool is_stopped(int axis_num);

    void move(float pos, int axis_num) override;
    void stop() override;

    void set_angle(float angle, int axis_num) override;
    void set_angle_range(float start_angle, float stop_angle, int points, int axis_num) override;

    int next_angle(int axis_num) override;
    int prev_angle(int axis_num) override;

    void move_to_start_angle(int axis_num) override;
    void move_to_stop_angle(int axis_num)override;

    float get_pos(int axis_num) override;

    int get_axes_count() override;
};


#endif //ANTESTL_BACKEND_TESART_RBD_HPP
