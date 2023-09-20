/**
 * \file
 * \brief Файл, в котором определён класс PlanarS50244
 *
 * \author Александр Горбунов
 * \date 19 сентября 2023
 */

#ifndef ANTESTL_BACKEND_PLANAR_S50244_H
#define ANTESTL_BACKEND_PLANAR_S50244_H

#include "vna_device.hpp"

/// Количество портов у ВАЦ Planar S50244
#define S50244_PORT_COUNT   2

/**
 * \brief Класс, в котором реализованы методы для работы с ВАЦ Planar S50244
 */
class PlanarS50244 : public VnaDevice {
/// Массив буквенных обозначений портов
    char port_names[S50244_PORT_COUNT]  = {'B', 'A'};
    /// Массив номеров портов
    int port_numbers[S50244_PORT_COUNT] = {2, 1};

public:
    PlanarS50244() = default;
    explicit PlanarS50244(std::string device_address);

    void preset() override;
    void full_preset() override;

    void init_channel() override;
    void configure(int meas_type, float rbw, int source_port) override;

    void create_traces(std::vector<int> port_list, bool external) override;

    void set_power(float power_) override;

    void set_freq_range(double start_freq, double stop_freq, int points) override;
    void set_freq(double freq) override;

    void set_path(std::vector<int> path_list) override;

    void rf_off() override;
    void rf_off(int port) override;

    void rf_on() override;
    void rf_on(int port) override;

    void trigger() override;

    void init() override;

    iq_port_data_t get_data(int trace_index) override;
};


#endif //ANTESTL_BACKEND_PLANAR_S50244_H
