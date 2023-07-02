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

#define COLUMN_DELIMITER    ","
#define ROW_DELIMITER       ";"

struct iq_port_list_t {
    std::vector<iq> iq_port_list;

    iq_port_list_t(iq iq_item) {
        insert_item(std::move(iq_item));
    }

    void insert_item(iq iq_item) {
        iq_port_list.push_back(std::move(iq_item));
    }

    std::string to_string() {
        std::string result{};

        for (const iq &item : iq_port_list) {
            result += (result.empty() ? "" : COLUMN_DELIMITER) + item.i + COLUMN_DELIMITER + item.q;
        }

        return result;
    }
};

struct data_t {
    std::vector<std::string> angle_list{};
    std::vector<double> freq_list{};

    std::vector<iq_port_list_t> iq_data_list{};

    void insert_angles(std::string angles) {
        angle_list.push_back(std::move(angles));
    }

    void insert_freq(double freq) {
        freq_list.push_back(freq);
    }

    void insert_freq_list(std::vector<double> freq_list) {
        this->freq_list = std::move(freq_list);
    }

    void insert_iq_port_data(iq_port_data_t iq_port_data) {
        if (iq_data_list.empty()) {
            for (const iq &item : iq_port_data) {
                iq_data_list.push_back(iq_port_list_t(item));
            }
        } else {
            for (int pos = 0; pos < iq_port_data.size(); ++pos) {
                iq_data_list[pos].insert_item(iq_port_data[pos]);
            }
        }
    }

    std::string to_string() {
        std::string result{};

        for (int pos = 0; pos < iq_data_list.size(); ++pos) {
            if (angle_list.size() == 1 && angle_list[0].empty()) {
                result +=
                        (result.empty() ? "" : ROW_DELIMITER) +
                        std::to_string(freq_list.size() > 1 ? freq_list[pos] : freq_list[0]) + COLUMN_DELIMITER +
                        iq_data_list[pos].to_string();
            } else {
                result +=
                        (result.empty() ? "" : ROW_DELIMITER) +
                        (angle_list.size() > 1 ? angle_list[pos] : angle_list[0]) + COLUMN_DELIMITER +
                        std::to_string(freq_list.size() > 1 ? freq_list[pos] : freq_list[0]) + COLUMN_DELIMITER +
                        iq_data_list[pos].to_string();
            }
        }

        return result;
    }
};

class DeviceSet {
    VnaDevice *vna = nullptr;
    GenDevice *ext_gen = nullptr;
    RbdDevice *rbd = nullptr;

    int meas_type = MEAS_TRANSITION;
    bool using_ext_gen = false;

    bool traces_configured = false;

    bool stop_requested = false;

public:
    DeviceSet() = default;

    bool connect(int device_type, std::string device_model, const std::string &device_address);
    void disconnect();

    bool configure(int meas_type, float rbw, int source_port, bool using_ext_gen);

    bool set_power(float power);

    bool set_freq(double freq);
    bool set_freq_range(double start_freq, double stop_freq, int points);

    int next_freq();
    int prev_freq();

    bool move_to_start_freq();

    double get_current_freq();
    std::vector<double> get_freq_list();

    bool set_angle(float angle, int axis_num);
    bool set_angle_range(float start_angle, float stop_angle, int points, int axis_num);

    int next_angle(int axis_num);
    int prev_angle(int axis_num);

    bool move_to_start_angle(int axis_num);

    std::string get_current_angles();

    bool set_path(std::vector<int> path_list);
    int get_vna_switch_module_count();

    bool is_using_ext_gen() const;

    data_t get_data(std::vector<int> port_list);

    void request_stop();

    void reset_stop_request();
};


#endif //ANTESTL_BACKEND_DEVICE_SET_HPP
