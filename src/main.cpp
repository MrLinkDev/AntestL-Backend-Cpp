#include <iostream>
#include "utils/logger.hpp"

#include "utils/socket/socket_server.hpp"
#include "devices/visa_device.hpp"
#include "devices/vna/keysight_m9807a.hpp"


int main() {
    logger::set_log_level(LEVEL_DEBUG);

    logger::log(LEVEL_ERROR, "Error msg");
    logger::log(LEVEL_INFO, "Info msg");
    logger::log(LEVEL_DEBUG, "Debug msg");
    logger::log(LEVEL_WARN, "Warn msg");

    std::string test = "12345";
    logger::log(LEVEL_DEBUG, "{} {}", test, 123);

    visa_device device("TCPIP0::K-N9020B-11111::inst0::INSTR");
    device.connect();
    std::cout << device.idn() << std::endl;
    std::cout << device.send_wait("*IDN?") << std::endl;

    VnaDevice *vna = new KeysightM9807A("TCPIP0::K-N9020B-11111::inst0::INSTR");

    int port_list[] = {2};

    vna->full_preset();
    vna->init_channel();
    vna->set_power(-2.23f);
    vna->set_freq(11e9, 13e9, 113);
    vna->create_traces(port_list, 1, false);
    //vna->trigger();
    iq_data_t iq_data = vna->get_data(0);

    for (int i = 0; i < 5; ++i) {
        std::cout << iq_data.at(i).first << " | " << iq_data.at(i).second << std::endl;
    }


    //int a[4] = {1, 3, 4, 2};
    //vna->set_path(a, 4);

    //int b[4] = {2, 2, 2, 2};
    //vna->set_path(b, 4);

    int c[5] = {1, 3, 5, 6, 7};
    std::cout << array_utils::index(c, 5, 2) << std::endl;
    std::cout << array_utils::index(c, 5, 6) << std::endl;

    return 0;
}
