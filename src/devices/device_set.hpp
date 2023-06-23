#ifndef ANTESTL_BACKEND_DEVICE_SET_HPP
#define ANTESTL_BACKEND_DEVICE_SET_HPP

#include <algorithm>
#include "vna/vna_device.hpp"
#include "gen/gen_device.hpp"
#include "rbd/rbd_device.hpp"
#include "vna/keysight_m9807a.hpp"
#include "gen/keysight_gen.hpp"
#include "rbd/tesart_rbd.hpp"

#define DEVICE_VNA  0xD0
#define DEVICE_GEN  0xD1
#define DEVICE_RBD  0xD2

#define DEFAULT_ANGLE       0.0

#define COLUMN_DELIMITER    ','
#define ROW_DELIMITER       ';'

struct data_struct_t {
    float angle;
    double freq;

    std::vector<std::string> port_i;
    std::vector<std::string> port_q;

    void insert_port_data(iq_data_item_t port_data) {
        port_i.push_back(port_data.first);
        port_q.push_back(port_data.second);
    }

    std::string to_string() {
        std::string out = std::format("{}{}{}", angle,COLUMN_DELIMITER , freq);

        for (int pos = 0; pos < port_i.size(); ++pos) {
            string_utils::join(&out, port_i[pos], COLUMN_DELIMITER);
            string_utils::join(&out, port_q[pos], COLUMN_DELIMITER);
        }

        return out;
    }
};

class DeviceSet {
    VnaDevice *vna = nullptr;
    GenDevice *ext_gen = nullptr;
    RbdDevice *rbd = nullptr;

    int meas_type = MEAS_TRANSITION;
    bool using_ext_gen = false;

public:
    DeviceSet() = default;

    int vna_switch_module_count() {
        if (vna != nullptr && vna->is_connected()) {
            return vna->get_switch_module_count();
        } else {
            return 0;
        }
    }

    bool connect(int device_type, std::string device_model, std::string device_address);
    bool configure(int meas_type, float rbw, int source_port, bool using_ext_gen);

    bool set_power(float power);

    bool set_freq(double freq);
    bool set_freq_range(double start_freq, double stop_freq, int points);

    bool next_freq();
    bool prev_freq();

    void move_to_start_freq();

    bool set_angle(float angle, int axis_num);
    bool set_angle_range(float start_angle, float stop_angle, int points, int axis_num);

    bool next_angle(int axis_num);
    bool prev_angle(int axis_num);

    void move_to_start_angle(int axis_num);

    bool change_path(std::vector<int> path_list);

    std::string get_data(std::vector<int> port_list, int axis_num);
};


#endif //ANTESTL_BACKEND_DEVICE_SET_HPP
