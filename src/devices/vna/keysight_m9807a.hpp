#ifndef ANTESTL_BACKEND_KEYSIGHT_M_9807_A_HPP
#define ANTESTL_BACKEND_KEYSIGHT_M_9807_A_HPP

#include "vna_device.hpp"

#define M9807A_PORT_COUNT   8
#define M9807A_SWITCH_COUNT 4

class KeysightM9807A : public VnaDevice {
    char port_names[M9807A_PORT_COUNT]  = {'D', 'B', 'H', 'F', 'C', 'E', 'G', 'A'};
    int port_numbers[M9807A_PORT_COUNT] = {4, 2, 8, 6, 3, 5, 7, 1};

    std::array<int, M9807A_SWITCH_COUNT> path_list = {1, 1, 1, 1};
public:
    KeysightM9807A() = default;
    KeysightM9807A(const std::string device_address);
    KeysightM9807A(visa_config config);

    void preset() override;
    void full_preset() override;

    void init_channel() override;
    void configure(int meas_type, double rbw, int source_port, bool ext_gen) override;

    void set_power(float power) override;
    void set_freq(double start, double stop, int points) override;
    void set_path(std::array<int, M9807A_SWITCH_COUNT> path_list);

    void get_data(int port_list) override;
};


#endif //ANTESTL_BACKEND_KEYSIGHT_M_9807_A_HPP
