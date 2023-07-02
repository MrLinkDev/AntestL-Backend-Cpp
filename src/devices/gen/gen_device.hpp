#ifndef ANTESTL_BACKEND_GEN_DEVICE_HPP
#define ANTESTL_BACKEND_GEN_DEVICE_HPP

#include "../visa_device.hpp"
#include "../../utils/exceptions.hpp"

#define DEFAULT_GEN_POWER           -20

#define DEFAULT_GEN_START_FREQ      1000000000
#define DEFAULT_GEN_STOP_FREQ       9000000000
#define DEFAULT_GEN_POINTS          201

#define DEFAULT_CURRENT_FREQ        0
#define DEFAULT_FREQ_STEP           0

#define DEFAULT_CURRENT_POINT       0

#define FREQ_MOVE_OK                0xF0
#define FREQ_MOVE_BOUND             0xF1

class GenDevice : public VisaDevice {

protected:
    float power         = DEFAULT_GEN_POWER;

    double start_freq   = DEFAULT_GEN_START_FREQ;
    double stop_freq    = DEFAULT_GEN_STOP_FREQ;

    double freq_step    = DEFAULT_FREQ_STEP;
    double current_freq = DEFAULT_CURRENT_FREQ;

    int points          = DEFAULT_GEN_POINTS;
    int current_point   = DEFAULT_CURRENT_POINT;

public:
    GenDevice() = default;

    explicit GenDevice(std::string device_address) : VisaDevice(std::move(device_address)) {
        this->connect();

        if (idn().empty()) {
            this->connected = false;

            logger::log(LEVEL_ERROR, NO_CONNECTION_MSG);
            throw antestl_exception(NO_CONNECTION_MSG, NO_CONNECTION_CODE);
        }
    }

    virtual void preset() {};

    virtual void set_freq(double freq) {};
    virtual void set_freq_range(double start_freq, double stop_freq, int points) {};

    virtual void set_power(float power) {};

    virtual void rf_off() {};
    virtual void rf_on() {};

    virtual int next_freq() {return FREQ_MOVE_BOUND;};
    virtual int prev_freq() {return FREQ_MOVE_BOUND;};

    virtual void move_to_start_freq() {};
    virtual void move_to_stop_freq() {};

    virtual double get_current_freq() {return 0.0;};
};

#endif //ANTESTL_BACKEND_GEN_DEVICE_HPP
