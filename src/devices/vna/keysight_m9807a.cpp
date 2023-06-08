#include <format>
#include "vna_device.hpp"

#define M9807A_PORT_COUNT   8

class KeysightM9807A : public VnaDevice {
    char port_names[8]  = {'D', 'B', 'H', 'F', 'C', 'E', 'G', 'A'};
    int port_numbers[8] = {4, 2, 8, 6, 3, 5, 7, 1};

public:
    KeysightM9807A() = default;

    KeysightM9807A(const string device_address) : VnaDevice(device_address) {}

    KeysightM9807A(visa_config config) : VnaDevice(config) {}

    void full_preset() override {
        this->send("*CLS");
        this->send("*RST");

        this->send(":SYSTEM:FPRESET");
        this->send(":DISPLAY:WINDOW1:STATE 1");
        this->send(":CALCULATE:PARAMETER:DELETE:ALL");

        this->send(":FORMAT:DATA ASCII,0");
    }

    void preset() override {
        this->send(":SYSTEM:PRESET");
    }

    void init_channel() override {
        this->send_wait_err(":CALCulate1:CUSTom:DEFine \"TR0\",\"Standard\",\"S11\"");
    }

    void configure(int meas_type, double rbw, int source_port, bool ext_gen) override {
        this->meas_type = meas_type;

        this->rbw = rbw;
        this->source_port = source_port;

        this->using_ext_gen = ext_gen;

        this->send_wait_err(":SENSE:ROSC:SOUR PXIBackplane");

        this->send_wait_err(":SENSe:SWEep:MODE HOLD");
        this->send_wait_err(format(":SENSe:BANDwidth:RESolution {}", this->rbw));
    }

    void set_power(float power) override {
        this->power = power;

        for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
            this->send_wait_err(format(":SOURce:POWer{}:LEVel:IMMediate:AMPLitude {},\"Port {}\"", port, this->power, port));
        }
    }

    void set_freq(double start, double stop, int points) override {
        this->start_freq = start;
        this->stop_freq = stop;
        this->points = points;

        this->send_wait_err(format(":SENSe:FREQuency:STARt {}", start));
        this->send_wait_err(format(":SENSe:FREQuency:STOP {}", stop));
        this->send_wait_err(format(":SENSe:SWEep:POINts {}", points));
    }
};
