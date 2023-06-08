#include "rbd_device.hpp"
#include <thread>
#include <format>

#include "../../utils/utils.hpp"

#define ADDRESS_DELIMITER   ';'

#define RBD_2_SLEEP_TIME_INIT   10000ms
#define RBD_2_SLEEP_TIME_WAIT   10000ms

#define BIT_REF_SET             0x00020000
#define BIT_IN_POS              0x00080000
#define BIT_MOVE_BLOCK          0x00010000

#define SCALE                   1000

class Rbd2 : public RbdDevice {
    VisaDevice *axes;
    int axis_count = 0;

    int velocity = 50;
    int acceleration = 3;

    int status(int axis_num = 0) {
        string str_answer{};
        int answer = 0;

        axes[axis_num].clear();

        try {
            axes[axis_num].query("TRJSTAT", &str_answer);
            str_answer = lstrip(str_answer, 'H');

            answer = stoi(str_answer);
        } catch (invalid_argument inv_arg) {
            axes[axis_num].query("TRJSTAT", &str_answer);
            str_answer = lstrip(str_answer, 'H');

            answer = stoi(str_answer);
        }

        return answer;
    }

public:
    Rbd2() = default;

    Rbd2(const string device_addresses) {
        axis_count = count(device_addresses, ADDRESS_DELIMITER);

        string *address_list;
        split(address_list, axis_count, device_addresses, ADDRESS_DELIMITER);

        axes = new VisaDevice[axis_count];
        for (int axis_num = 0; axis_num < axis_count; ++axis_num) {
            axes[axis_num] = VisaDevice(address_list[axis_num]);

            axes[axis_num].write("PROMPT 0");
            axes[axis_num].write("CLRFAULT");
            axes[axis_num].write("EN");

            axes[axis_num].clear();
            std::this_thread::sleep_for(RBD_2_SLEEP_TIME_INIT);

            if (!(status(axis_num) & BIT_REF_SET)) {
                axes[axis_num].write("MH");
            }
        }


    }

    bool is_stopped(int axis_num) {
        int status_code = status(axis_num);

        return bool(status_code & BIT_IN_POS) and bool(!(status_code & BIT_MOVE_BLOCK));
    }

    void move(float pos, int axis_num) {
        logger::log(LEVEL_INFO, "Axis ", to_string(axis_num).c_str(), " (", to_string(pos).c_str(), ")", NULL);

        axes[axis_num].write(
                format(
                        "ORDER 0 {} {} 8192 {} {} 0 -1 0 0",
                        int(pos * SCALE), velocity, acceleration, acceleration));

        axes[axis_num].write("MOVE 0");
    }

    float get_position(int axis_num) {
        axes[axis_num].clear();

        string str_answer{};
        float answer = 0;

        axes[axis_num].query("PFB", &str_answer);
        answer = stof(str_answer) / SCALE;

        return answer;
    }

    void wait(int axis_num) {
        while (!is_stopped(axis_num)) {
            std::this_thread::sleep_for(RBD_2_SLEEP_TIME_WAIT);
        }
    }
};
