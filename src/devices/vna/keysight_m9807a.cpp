#include "keysight_m9807a.hpp"

KeysightM9807A::KeysightM9807A(const std::string device_address) : VnaDevice(device_address) {}

KeysightM9807A::KeysightM9807A(visa_config config) : VnaDevice(config) {}

void KeysightM9807A::preset() {
    send(":SYSTEM:PRESET");
}

void KeysightM9807A::full_preset() {
    send("*CLS");
    send("*RST");

    send(":SYSTEM:FPRESET");
    send(":DISPLAY:WINDOW1:STATE 1");
    send(":CALCULATE:PARAMETER:DELETE:ALL");

    send(":FORMAT:DATA ASCII,0");
}

void KeysightM9807A::init_channel() {
    send_wait_err(":CALCulate1:CUSTom:DEFine \"TR0\",\"Standard\",\"S11\"");
}

void KeysightM9807A::configure(int meas_type, double rbw, int source_port, bool ext_gen) {
    this->meas_type = meas_type;

    this->rbw = rbw;
    this->source_port = source_port;

    send_wait_err(":SENSE:ROSC:SOUR PXIBackplane");

    send_wait_err(":SENSe:SWEep:MODE HOLD");
    send_wait_err(":SENSe:BANDwidth:RESolution {}", this->rbw);
}

void KeysightM9807A::set_power(float power) {
    this->power = power;

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:LEVel:IMMediate:AMPLitude {},\"Port {}\"", port, this->power, port);
    }
}

void KeysightM9807A::set_freq(double start, double stop, int points) {
    this->start_freq = start;
    this->stop_freq = stop;
    this->points = points;

    send_wait_err(":SENSe:FREQuency:STARt {}", this->start_freq);
    send_wait_err(":SENSe:FREQuency:STOP {}", this->stop_freq);
    send_wait_err(":SENSe:SWEep:POINts {}", this->points);
}

void KeysightM9807A::set_path(int *path_list, int module_count = M9807A_MODULE_COUNT) {
    for (int mod = 0; mod < M9807A_MODULE_COUNT; ++mod) {
        if (this->path_list[mod] == path_list[mod]) {
            continue;
        }

        send_wait_err("SENS:SWIT:M9157:MOD{}:SWIT:PATH STAT{}", mod + 1, path_list[mod]);
        this->path_list[mod] = path_list[mod];
    }

    send_wait_err("INIT");
}

void KeysightM9807A::get_data(int port_list) {

}