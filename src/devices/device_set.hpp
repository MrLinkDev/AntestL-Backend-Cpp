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

class DeviceSet {
    VnaDevice *vna{};
    GenDevice *ext_gen{};
    RbdDevice *rbd{};

public:
    DeviceSet() = default;

    bool connect(int device_type, std::string device_model, std::string device_address);
    bool configure(int meas_type, float rbw, int source_port, bool using_ext_gen);

    bool set_power(float power);

    bool set_freq(double freq);
    bool set_freq_range(double start_freq, double stop_freq, int points);

    void next_freq();
    void prev_freq();
};


#endif //ANTESTL_BACKEND_DEVICE_SET_HPP
