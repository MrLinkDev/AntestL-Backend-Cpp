#ifndef ANTESTL_BACKEND_KEYSIGHT_GEN_HPP
#define ANTESTL_BACKEND_KEYSIGHT_GEN_HPP


#include "gen_device.hpp"

class KeysightGen : public GenDevice {

public:
    KeysightGen() = default;
    KeysightGen(const std::string device_address);
    KeysightGen(visa_config config);

    void preset() override;

    void set_freq(double freq) override;
    void set_freq_range(double start, double stop, int points) override;

    void set_power(float value) override;

    void rf_off() override;
    void rf_on() override;

    void next_freq() override;
    void prev_freq() override;

    void move_to_start_freq() override;
    void move_to_stop_freq() override;

    double get_current_freq() override;
};


#endif //ANTESTL_BACKEND_KEYSIGHT_GEN_HPP
