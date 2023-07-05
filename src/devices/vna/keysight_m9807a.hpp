/**
 * \file
 * \brief Файл, в котором определён класс KeysightM9807A
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_KEYSIGHT_M_9807_A_HPP
#define ANTESTL_BACKEND_KEYSIGHT_M_9807_A_HPP

#include "vna_device.hpp"

/// Количество портов у ВАЦ Keysight M9807A
#define M9807A_PORT_COUNT   8
/// Количество модулей переключателей, подключенных к M9807A
#define M9807A_MODULE_COUNT 4

/**
 * \brief Класс, в котором реализованы методы для работы с ВАЦ Keysight M9807A
 */
class KeysightM9807A : public VnaDevice {
    /// Массив буквенных обозначений портов
    char port_names[M9807A_PORT_COUNT]  = {'D', 'B', 'H', 'F', 'C', 'E', 'G', 'A'};
    /// Массив номеров портов
    int port_numbers[M9807A_PORT_COUNT] = {4, 2, 8, 6, 3, 5, 7, 1};

    /// Массив, в котором хранится набор состояний переключателей
    std::array<int, M9807A_MODULE_COUNT> path_list = {1, 1, 1, 1};
public:
    KeysightM9807A() = default;
    explicit KeysightM9807A(std::string device_address);

    void preset() override;
    void full_preset() override;

    void init_channel() override;
    void configure(int meas_type, float rbw, int source_port) override;

    void create_traces(std::vector<int> port_list, bool external) override;

    void set_power(float power_) override;

    void set_freq_range(double start_freq, double stop_freq, int points) override;
    void set_freq(double freq) override;

    void set_path(std::vector<int> path_list) override;
    int get_switch_module_count() override;

    void rf_off() override;
    void rf_off(int port) override;

    void rf_on() override;
    void rf_on(int port) override;

    void trigger() override;

    void init() override;

    iq_port_data_t get_data(int trace_index) override;
};


#endif //ANTESTL_BACKEND_KEYSIGHT_M_9807_A_HPP
