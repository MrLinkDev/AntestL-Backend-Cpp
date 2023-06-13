#ifndef ANTESTL_BACKEND_VNA_DEVICE_HPP
#define ANTESTL_BACKEND_VNA_DEVICE_HPP

#include "../visa_device.hpp"

#define DEFAULT_VNA_START_FREQ      1000000000
#define DEFAULT_VNA_STOP_FREQ       9000000000

#define DEFAULT_VNA_RBW             1000

#define DEFAULT_VNA_POINTS          201

#define DEFAULT_VNA_POWER           -20

#define DEFAULT_VNA_SOURCE_PORT     1

#define MEAS_TRANSITION             0x00
#define MEAS_REFLECTION             0x01

class VnaDevice : public VisaDevice {

protected:
    double start_freq   = DEFAULT_VNA_START_FREQ;
    double stop_freq    = DEFAULT_VNA_STOP_FREQ;

    int points          = DEFAULT_VNA_POINTS;

    float rbw           = DEFAULT_VNA_RBW;

    float power         = DEFAULT_VNA_POWER;

    int source_port     = DEFAULT_VNA_SOURCE_PORT;

    int meas_type       = MEAS_TRANSITION;

public:
    VnaDevice() = default;
    VnaDevice(const std::string device_address) : VisaDevice(device_address){
        if (idn().empty()) {
            this->connected = false;
        }
    }
    VnaDevice(visa_config config) : VisaDevice(config) {
        if (idn().empty()) {
            this->connected = false;
        }
    }

    virtual void preset() = 0;
    virtual void full_preset() = 0;

    virtual void init_channel() = 0;
    virtual void configure(int meas_type, double rbw, int source_port, bool ext_gen) = 0;

    virtual void set_power(float power) = 0;
    virtual void set_freq(double start, double stop, int points) = 0;
    virtual void set_path(int path_list) = 0;

    virtual void get_data(int port_list) = 0;
};


#endif //ANTESTL_BACKEND_VNA_DEVICE_HPP
