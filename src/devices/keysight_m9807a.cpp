#include "../utils/visa_device.hpp"



class KeysightM9807A : public VisaDevice {
    char port_names[7]  = {'D', 'B', 'H', 'F', 'C', 'E', 'G'};
    int port_numbers[7] = {4, 2, 8, 6, 3, 5, 7};

    float power         = DEFAULT_POWER;
    float rbw           = DEFAULT_RBW;
    int source_port     = DEFAULT_SOURCE_PORT;

    bool using_ext_gen  = false;

public:
    KeysightM9807A() = default;

    KeysightM9807A(const string device_address) : VisaDevice(device_address) {

    }

    KeysightM9807A(visa_config config) : VisaDevice(config) {

    }

    int full_preset() override {
        this->send("*CLS");
        this->send("*RST");

        this->send(":SYSTEM:FPRESET");
        this->send(":DISPLAY:WINDOW1:STATE 1");
        this->send(":CALCULATE:PARAMETER:DELETE:ALL");

        this->send(":FORMAT:DATA ASCII,0");
    }

    int preset() override {
        this->send(":SYSTEM:PRESET");
    }

    int configure(float rbw = DEFAULT_RBW, int source_port = DEFAULT_SOURCE_PORT) override {
        this->rbw = rbw;
        this->source_port = source_port;
    }
};
