/**
 * \file
 * \brief Заголовочный файл, в котором определён класс KeysightGen
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_KEYSIGHT_GEN_HPP
#define ANTESTL_BACKEND_KEYSIGHT_GEN_HPP

#include "gen_device.hpp"

/**
 * \brief Класс KeysightGen, в котором определены методы для работы с генераторами
 * Keysight
 */
class KeysightGen : public GenDevice {

public:
    KeysightGen() = default;
    explicit KeysightGen(std::string device_address);

    void preset() override;

    void set_freq(double freq) override;
    void set_freq_range(double start_freq, double stop_freq, int points) override;

    void set_power(float power) override;

    void rf_off() override;
    void rf_on() override;

    int next_freq() override;
    int prev_freq() override;

    void move_to_start_freq() override;
    void move_to_stop_freq() override;

    double get_current_freq() override;
};


#endif //ANTESTL_BACKEND_KEYSIGHT_GEN_HPP
