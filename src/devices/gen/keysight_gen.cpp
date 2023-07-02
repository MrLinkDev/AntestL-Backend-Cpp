#include "keysight_gen.hpp"

KeysightGen::KeysightGen(std::string device_address) : GenDevice(std::move(device_address)) {}

void KeysightGen::preset() {
    send("SYSTEM:PRESET");

    send(":MODULATION:STATE OFF");
    send("OUTPUT:STATE OFF");
}

void KeysightGen::set_freq(double freq) {
    start_freq = freq;
    stop_freq = freq;

    current_freq = freq;

    points = 1;
    current_point = 0;

    send_wait(":FREQ {}", current_freq);
}

void KeysightGen::set_freq_range(double start_freq, double stop_freq, int points) {
    this->start_freq = start_freq;
    this->stop_freq = stop_freq;

    this->points = points;

    freq_step = this->points <= 1 ? 0 : (this->stop_freq - this->start_freq) / (this->points - 1);

    current_freq = start_freq;
    current_point = 0;
}

void KeysightGen::set_power(float power) {
    this->power = power;

    send(":SOURCE:POWER {}", this->power);
}

void KeysightGen::rf_off() {
    send("OUTPUT:STATE OFF");
}

void KeysightGen::rf_on() {
    send("OUTPUT:STATE ON");
}

int KeysightGen::next_freq() {
    if (current_point == points - 1) {
        return FREQ_MOVE_BOUND;
    }

    rf_off();

    current_freq += freq_step;
    ++current_point;

    send_wait(":FREQ {}", current_freq);
    send(":FREQ?");

    return FREQ_MOVE_OK;
}

int KeysightGen::prev_freq() {
    if (current_point == 0) {
        return FREQ_MOVE_BOUND;
    }

    rf_off();

    current_freq -= freq_step;
    --current_point;

    send_wait(":FREQ {}", current_freq);

    return FREQ_MOVE_OK;
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

