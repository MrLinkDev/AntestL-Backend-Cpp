#ifndef ANTESTL_BACKEND_DEVICE_SET_HPP
#define ANTESTL_BACKEND_DEVICE_SET_HPP

#include <algorithm>
#include <json.hpp>
#include "visa_device.hpp"

using namespace nlohmann;
using namespace std;

class DeviceSet {
    VisaDevice *vna;
    VisaDevice *external_gen;
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
        auto meas_type      = params["meas_type"].get<int>();
        auto rbw            = params["rbw"].get<float>();
        auto source_port    = params["source_port"].get<int>();
        auto external           = params["external"].get<bool>();

        vna->configure(meas_type, rbw, source_port, external);

        return true;
    }
};

#endif //ANTESTL_BACKEND_DEVICE_SET_HPP
