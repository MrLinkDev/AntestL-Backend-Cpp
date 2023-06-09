#ifndef ANTESTL_BACKEND_DEVICE_SET_HPP
#define ANTESTL_BACKEND_DEVICE_SET_HPP

#include <algorithm>
#include "json.hpp"

#include "visa_device.hpp"
#include "../devices/ext_gen/ext_gen_device.hpp"

using namespace nlohmann;
using namespace std;

class DeviceSet {
    VnaDevice *vna;
    ExtGenDevice *external_gen;
    VisaDevice *rbd;

public:
    DeviceSet() = default;

    bool connect_to_vna(string device_model, string device_address) {
        transform(device_model.begin(), device_model.end(), device_model.begin(), ::toupper);

        if (device_model == "M9807A") {
            vna = new KeysightM9807A(device_address);
        }

        return vna->is_exist();
    }

    bool connect_to_ext_gen(string device_address) {
        //TODO: Create external gen object

        return external_gen->is_exist();
    }

    bool connect_to_rbd(string azimuth, string elevation, string roll) {
        //TODO: Create rbd object

        return rbd->is_exist();
    }

    bool configure(json params) {
        auto meas_type = params["meas_type"].get<int>();
        auto rbw = params["rbw"].get<float>();
        auto source_port = params["source_port"].get<int>();
        auto external = params["external"].get<bool>();

        int result;

        try {
            vna->full_preset();
            logger::log(LEVEL_DEBUG, "Made full preset", NULL);

            vna->init_channel();
            logger::log(LEVEL_DEBUG, "Default channel initialized", NULL);

            vna->configure(meas_type, rbw, source_port, external);
            logger::log(LEVEL_DEBUG, "VNA configured", NULL);
        } catch (int code) {
            return false;
        }

        return true;
    }

    bool set_power(json params) {
        auto power = params["value"].get<float>();

        try {
            if (vna->using_ext_gen) {
                external_gen->set_power(power);
            } else {
                vna->set_power(power);
            }
        } catch (int code) {
            return false;
        }

        return true;
    }

    bool set_freq(json params) {
        auto start_freq = params["start"].get<float>();
        auto stop_freq = params["stop"].get<float>();
        auto points = params["points"].get<int>();

        try {
            if (vna->using_ext_gen) {
                external_gen->set_freq(start_freq, stop_freq, points);
            } else {
                vna->set_freq(start_freq, stop_freq, points);
            }
        } catch (int code) {
            return false;
        }

        return true;
    }

    bool set_angle(json params) {
        return false;
    }

    bool change_path(json params) {
        return false;
    }

    json get_data(json params) {
        return json();
    }
};

#endif //ANTESTL_BACKEND_DEVICE_SET_HPP
