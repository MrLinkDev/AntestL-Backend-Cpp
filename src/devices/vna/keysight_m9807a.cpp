#include "keysight_m9807a.hpp"
#include "../../utils/string_utils.hpp"
#include "../../utils/array_utils.hpp"

KeysightM9807A::KeysightM9807A(std::string device_address) : VnaDevice(std::move(device_address)) {}

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
    send_wait_err(R"(:CALCulate1:CUSTom:DEFine "TR0","Standard","S11")");

    send_wait_err(":SOURce:POWer:COUPle 1");

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:MODE OFF", port);
    }

    send_wait_err(":SOURce:POWer:COUPle 0");
}

void KeysightM9807A::configure(int meas_type, float rbw, int source_port) {
    this->meas_type = meas_type;

    this->rbw = rbw;
    this->source_port = source_port;

    send_wait_err(":SENSE:ROSC:SOUR PXIBackplane");

    send_wait_err(":SENSe:SWEep:MODE HOLD");
    send_wait_err(":SENSe:BANDwidth:RESolution {}", this->rbw);
}

void KeysightM9807A::create_traces(std::vector<int> port_list, bool external) {
    send_wait_err(":CALCulate:PARameter:DELete:ALL");

    std::string trace_name{};
    std::string trace_params{};

    int trace_arg = external ? 0 : source_port;

    for (int pos = 0; pos < port_list.size(); ++pos) {
        if (external) {
            char port_name = port_names[
                    array_utils::index(
                            port_numbers,
                            M9807A_PORT_COUNT,
                            port_list[pos]
                    )];

            char source_port_name = port_names[
                    array_utils::index(
                            port_numbers,
                            M9807A_PORT_COUNT,
                            port_list[source_port]
                    )];

            if (meas_type == MEAS_TRANSITION) {
                trace_name = std::format("{}/{}", port_name, source_port_name);
                trace_params = std::format("{},{}", trace_name, trace_arg);
            } else {
                trace_name = trace_params = std::format("S{}{}", port_list[pos], port_list[pos]);
            }
        } else {
            if (meas_type == MEAS_TRANSITION) {
                trace_name = trace_params = std::format("S{}{}", port_list[pos], source_port);
            } else {
                trace_name = trace_params = std::format("S{}{}", port_list[pos], port_list[pos]);
            }
        }

        send_wait_err(R"(:CALCulate1:CUSTom:DEFine "TR{}","Standard","{}")", port_list[pos], trace_params);
        send_wait_err(":DISPlay:WINDow:TRACe{}:FEED \"TR{}\"", port_list[pos], port_list[pos]);
    }
}

void KeysightM9807A::set_power(float power) {
    this->power = power;

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:LEVel:IMMediate:AMPLitude {},\"Port {}\"", port, this->power, port);
    }
}

void KeysightM9807A::set_freq_range(double start_freq, double stop_freq, int points) {
    this->start_freq = start_freq;
    this->stop_freq = stop_freq;
    this->points = points;

    freq_step = this->points <= 1 ? 0 : (this->stop_freq - this->start_freq) / (this->points - 1);

    send_wait_err(":SENSe:SWEep:POINts {}", this->points);

    send_wait_err(":SENSe:FREQuency:STARt {}", this->start_freq);
    send_wait_err(":SENSe:FREQuency:STOP {}", this->stop_freq);
}

void KeysightM9807A::set_freq(double freq) {
    start_freq = freq;
    stop_freq = freq;
    points = 1;

    freq_step = 0;

    send_wait_err(":SENSe:SWEep:POINts {}", points);

    send_wait_err(":SENSe:FREQuency:STARt {}", start_freq);
    send_wait_err(":SENSe:FREQuency:STOP {}", stop_freq);
}

void KeysightM9807A::set_path(std::vector<int> path_list) {
    for (int mod = 0; mod < M9807A_MODULE_COUNT; ++mod) {
        if (path_list[mod] == this->path_list[mod] || path_list[mod] == -1) {
            continue;
        }

        this->path_list[mod] = path_list[mod];
        send_wait_err("SENS:SWIT:M9157:MOD{}:SWIT:PATH STAT{}", mod + 1, this->path_list[mod]);
    }

    send_wait_err("INIT");
}

void KeysightM9807A::rf_off() {
    send_wait_err(":SOURce:POWer:COUPle 0");

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:MODE OFF", port);
    }
}

void KeysightM9807A::rf_off(int port) {
    send_wait_err(":SOURce:POWer:COUPle 0");
    send_wait_err(":SOURce:POWer{}:MODE OFF", port);
}

void KeysightM9807A::rf_on() {
    send_wait_err(":SOURce:POWer:COUPle 0");

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:MODE ON", port);
    }
}

void KeysightM9807A::rf_on(int port) {
    send_wait_err(":SOURce:POWer:COUPle 0");
    send_wait_err(":SOURce:POWer{}:MODE ON", port);
}

void KeysightM9807A::trigger() {
    send_wait_err(":TRIGger:SEQuence:SOURce MANual");
    send_wait_err(":TRIGger:SEQuence:SCOPe CURRent");

    send_wait_err("TRIG:SCOP ALL");
}

void KeysightM9807A::init() {
    send_wait_err("INIT");
}


iq_port_data_t KeysightM9807A::get_data(int trace_index) {
    iq_port_data_t iq_data{};

    std::string received_data = send(":CALCULATE:MEASURE{}:DATA:SDATA?", trace_index + 1);
    std::vector<std::string> cached_data = string_utils::split(received_data, DATA_DELIMITER);

    for (int pos = 0; pos < cached_data.size(); pos += 2) {
        iq_data.emplace_back(cached_data[pos], cached_data[pos + 1]);
    }

    return iq_data;
}

int KeysightM9807A::get_switch_module_count() {
    return M9807A_MODULE_COUNT;
}