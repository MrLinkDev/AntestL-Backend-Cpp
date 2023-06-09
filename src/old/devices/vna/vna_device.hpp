#ifndef ANTESTL_BACKEND_VNADEVICE_HPP
#define ANTESTL_BACKEND_VNADEVICE_HPP

#include "../../utils/visa_device.hpp"

#define DEFAULT_VNA_POWER           -20
#define DEFAULT_VNA_RBW             1000
#define DEFAULT_VNA_SOURCE_PORT     1

#define DEFAULT_VNA_START_FREQ      1000000000
#define DEFAULT_VNA_STOP_FREQ       9000000000
#define DEFAULT_VNA_POINTS          201

#define MEAS_TRANSITION         0x00
#define MEAS_REFLECTION         0x01

class VnaDevice : public VisaDevice {
protected:
    int meas_type       = MEAS_TRANSITION;

    float power         = DEFAULT_VNA_POWER;
    float rbw           = DEFAULT_VNA_RBW;

    int source_port     = DEFAULT_VNA_SOURCE_PORT;

    double start_freq   = DEFAULT_VNA_START_FREQ;
    double stop_freq    = DEFAULT_VNA_STOP_FREQ;

    int points          = DEFAULT_VNA_POINTS;

public:
    VnaDevice() = default;

    VnaDevice(const string device_address) : VisaDevice(device_address) {
        if (idn() == FAILURE) {
            this->exist = false;
        }
    }

    VnaDevice(visa_config config) : VisaDevice(config) {
        if (idn() == FAILURE) {
            this->exist = false;
        }
    }

    virtual void full_preset() {
        logger::log(LEVEL_DEBUG, "Called \"full_preset\" function");
    }

    virtual void preset() {
        logger::log(LEVEL_DEBUG, "Called \"preset\" function");
    }

    virtual void init_channel() {
        logger::log(LEVEL_DEBUG, "Called \"init_channel\" function");
    }

    virtual void configure(int meas_type, double rbw, int source_port, bool ext_gen) {
        logger::log(LEVEL_DEBUG, "Called \"configure\" function");
    }

    virtual void set_power(float power) {
        logger::log(LEVEL_DEBUG, "Called \"set_power\" function");
    }

    virtual void set_freq(double start, double stop, int points) {
        logger::log(LEVEL_DEBUG, "Called \"set_freq\" function");
    }

};

#endif //ANTESTL_BACKEND_VNADEVICE_HPP
