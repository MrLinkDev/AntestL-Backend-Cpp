#ifndef ANTESTL_BACKEND_VNA_DEVICE_HPP
#define ANTESTL_BACKEND_VNA_DEVICE_HPP

#include "../visa_device.hpp"
#include "../../utils/exceptions.hpp"

#define DEFAULT_VNA_START_FREQ      1000000000
#define DEFAULT_VNA_STOP_FREQ       9000000000

#define DEFAULT_VNA_RBW             1000

#define DEFAULT_VNA_POINTS          201

#define DEFAULT_VNA_POWER           -20

#define DEFAULT_VNA_SOURCE_PORT     1

#define MEAS_TRANSITION             0x00
#define MEAS_REFLECTION             0x01

#define DATA_DELIMITER              ','

typedef std::pair<std::string, std::string> iq_data_item_t;
typedef std::vector<iq_data_item_t> iq_data_t;

class VnaDevice : public VisaDevice {

protected:
    double start_freq   = DEFAULT_VNA_START_FREQ;
    double stop_freq    = DEFAULT_VNA_STOP_FREQ;
    double freq_step    = 0;

    int points          = DEFAULT_VNA_POINTS;

    float rbw           = DEFAULT_VNA_RBW;

    float power         = DEFAULT_VNA_POWER;

    int source_port     = DEFAULT_VNA_SOURCE_PORT;

    int meas_type       = MEAS_TRANSITION;

public:
    VnaDevice() = default;

    VnaDevice(const std::string device_address) : VisaDevice(device_address){
        this->connect();

        if (idn().empty()) {
            this->connected = false;
            throw NO_CONNECTION;
        }
    }

    VnaDevice(visa_config config) : VisaDevice(config) {
        this->connect();

        if (idn().empty()) {
            this->connected = false;
            throw NO_CONNECTION;
        }
    }

    int get_source_port() {
        return source_port;
    };

    double get_freq_by_point_num(int point_num) {
        logger::log(LEVEL_DEBUG, "Freq step = {}", freq_step);
        return start_freq + freq_step * point_num;
    }

    int get_points() {
        return points;
    }

    virtual void preset() {};
    virtual void full_preset() {};

    virtual void init_channel() {};
    virtual void configure(int meas_type, double rbw, int source_port, bool ext_gen) {};

    virtual void create_traces(int *port_list, int length, bool external) {};

    virtual void set_power(float power) {};
    virtual void set_freq_range(double start, double stop, int points) {};
    virtual void set_freq(double freq) {};

    virtual void set_path(int *path_list, int module_count) {};

    virtual void rf_off() {};
    virtual void rf_off(int port) {};

    virtual void rf_on() {};
    virtual void rf_on(int port) {};

    virtual void trigger() {};

    virtual void init() {};

    virtual iq_data_t get_data(int trace_index) {return iq_data_t{};};
};


#endif //ANTESTL_BACKEND_VNA_DEVICE_HPP
