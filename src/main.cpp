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

    VnaDevice *vna = new KeysightM9807A("TCPIP0::K-N9020B-11111::inst0::INSTR");

    int port_list[] = {2};

    vna->full_preset();
    vna->init_channel();
    vna->set_power(-2.23f);
    vna->set_freq(11e9, 13e9, 113);
    vna->create_traces(port_list, 1, false);
    vna->trigger();
    iq_data_t iq_data = vna->get_data(0);

    return 0;
}
