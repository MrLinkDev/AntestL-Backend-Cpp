#include "keysight_gen.hpp"

KeysightGen::KeysightGen(const std::string device_address) : GenDevice(device_address) {}

KeysightGen::KeysightGen(visa_config config) : GenDevice(config) {}

void KeysightGen::preset() {
    send("SYSTEM:PRESET");

    send(":MODULATION:STATE OFF");
    send("OUTPUT:STATE OFF");
}

void KeysightGen::set_freq(double freq) {
    this->start_freq = freq;
    this->stop_freq = freq;

    this->current_freq = freq;

    this->points = 1;
    this->current_point = 0;

    send_wait(":FREQ {}", current_freq);
}

void KeysightGen::set_freq_range(double start, double stop, int points) {
    this->start_freq = start;
    this->stop_freq = stop;

    this->points = points;

    freq_step = this->points <= 1 ? 0 : (this->stop_freq - this->start_freq) / (this->points - 1);

    current_freq = start_freq;
    current_point = 0;
}

void KeysightGen::set_power(float value) {
    send(":SOURCE:POWER {}", value);
}

void KeysightGen::rf_off() {
    send("OUTPUT:STATE OFF");
}

void KeysightGen::rf_on() {
    send("OUTPUT:STATE ON");
}

void KeysightGen::next_freq() {
    if (current_point == points - 1) {
        throw FREQ_OUT_OF_BOUND;
    }

    rf_off();

    current_freq += freq_step;
    ++current_point;

    send_wait(":FREQ {}", current_freq);
    send(":FREQ?");
}

void KeysightGen::prev_freq() {
    if (current_point == 0) {
        throw FREQ_OUT_OF_BOUND;
    }

    rf_off();

    current_freq -= freq_step;
    --current_point;

    send_wait(":FREQ {}", current_freq);
}

void KeysightGen::move_to_start_freq() {
    rf_off();

    current_freq = start_freq;
    current_point = 0;

    send_wait(":FREQ {}", current_freq);
}

void KeysightGen::move_to_stop_freq() {
    rf_off();

    current_freq = stop_freq;
    current_point = points - 1;

    send_wait(":FREQ {}", current_freq);
}

double KeysightGen::get_current_freq() {
    std::string current_freq = send(":FREQ?");
    return std::stod(current_freq);
}

