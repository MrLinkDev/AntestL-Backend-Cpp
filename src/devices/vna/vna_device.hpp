#ifndef ANTESTL_BACKEND_VNA_DEVICE_HPP
#define ANTESTL_BACKEND_VNA_DEVICE_HPP

#include "../visa_device.hpp"
#include "../../utils/exceptions.hpp"

#include <vector>

#define DEFAULT_VNA_START_FREQ      1000000000
#define DEFAULT_VNA_STOP_FREQ       9000000000

#define DEFAULT_VNA_RBW             1000

#define DEFAULT_VNA_POINTS          201

#define DEFAULT_VNA_POWER           -20;

#define DEFAULT_VNA_SOURCE_PORT     1

#define MEAS_TRANSITION             0x00
#define MEAS_REFLECTION             0x01

#define DATA_DELIMITER              ','

struct iq {
    std::string i{};
    std::string q{};
};

typedef std::vector<iq> iq_port_data_t;

class VnaDevice : public VisaDevice {

protected:
    double start_freq = DEFAULT_VNA_START_FREQ;
    double stop_freq = DEFAULT_VNA_STOP_FREQ;

    float rbw = DEFAULT_VNA_RBW;
    int points = DEFAULT_VNA_POINTS;

    double freq_step = (stop_freq - start_freq) / (points - 1);

    float power = DEFAULT_VNA_POWER;

    int source_port = DEFAULT_VNA_SOURCE_PORT;
    int meas_type = MEAS_TRANSITION;

public:
    VnaDevice() = default;

    explicit VnaDevice(std::string device_address) : VisaDevice(std::move(device_address)){
        this->connect();

        if (!this->is_connected() || idn().empty()) {
            this->connected = false;

            logger::log(LEVEL_ERROR, NO_CONNECTION_MSG);
            throw antestl_exception(NO_CONNECTION_MSG, NO_CONNECTION_CODE);
        }
    }

    virtual void preset() {};
    virtual void full_preset() {};

    virtual void init_channel() {};
    virtual void configure(int meas_type, float rbw, int source_port) {};

    virtual void create_traces(std::vector<int> port_list, bool external) {};

    virtual void set_power(float power) {};

    virtual void set_freq_range(double start_freq, double stop_freq, int points) {};
    virtual void set_freq(double freq) {};

    virtual void set_path(std::vector<int> path_list) {};
    virtual int get_switch_module_count() {return 0;}

    virtual void rf_off() {};
    virtual void rf_off(int port) {};

    virtual void rf_on() {};
    virtual void rf_on(int port) {};

    virtual void trigger() {};

    virtual void init() {};

    virtual iq_port_data_t get_data(int trace_index) {return iq_port_data_t{};};

    int get_source_port() const {
        return source_port;
    };

    double get_start_freq() const {
        return start_freq;
    }

    double get_freq_step() const {
        return freq_step;
    }

    int get_points() const {
        return points;
    }
};


#endif //ANTESTL_BACKEND_VNA_DEVICE_HPP
