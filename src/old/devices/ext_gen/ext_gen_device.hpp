#include <format>
#include "../../utils/visa_device.hpp"

#define DEFAULT_GEN_POWER           -20

#define DEFAULT_GEN_START_FREQ      1000000000
#define DEFAULT_GEN_STOP_FREQ       9000000000
#define DEFAULT_GEN_POINTS          201

#define FREQ_RANGE_OUT_OF_BOUND     0xFF

class ExtGenDevice : public VisaDevice {
    float power = DEFAULT_GEN_POWER;

    double start_freq = DEFAULT_GEN_START_FREQ;
    double stop_freq = DEFAULT_GEN_STOP_FREQ;

    int points = DEFAULT_GEN_POINTS;

    double current_freq = 0;
    double step_freq = 0;

    double current_point = 0;

public:
    ExtGenDevice() = default;

    ExtGenDevice(const string device_address) : VisaDevice(device_address) {}

    ExtGenDevice(visa_config config) : VisaDevice(config) {}

    virtual void rf_off() {
        this->send("OUTPUT:STATE OFF");
    }

    virtual void rf_on() {
        this->send("OUTPUT:STATE ON");
    }

    virtual void set_power(float value) {
        this->send(":MODULATION:STATE OFF");
        this->send(std::format(":SOURCE:POWER {}", value));
    }

    virtual void set_freq(double start, double stop, int points) {
        this->start_freq = start;
        this->stop_freq = stop;
        this->points = points;

        this->step_freq = this->points <= 1 ? 0 : (this->start_freq - this->stop_freq) / (this->points - 1);

        this->current_freq = this->start_freq;
        this->current_point = 0;
    }

    virtual void sweep() {
        this->current_freq += this->step_freq;
        this->current_point += 1;

        if (this->current_point >= this->points || this->current_freq > this->stop_freq) {
            throw FREQ_RANGE_OUT_OF_BOUND;
        }
    }

};

