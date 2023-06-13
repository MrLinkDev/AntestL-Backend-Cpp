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

    std::string test = "12345";
    logger::log(LEVEL_DEBUG, "{} {}", test, 123);

    visa_device device("TCPIP0::K-N9020B-11111::inst0::INSTR");
    device.connect();
    std::cout << device.idn() << std::endl;
    std::cout << device.send_wait("*IDN?") << std::endl;

    try {
        std::cout << device.send_wait("*IDNA?") << std::endl;
    } catch (int error_code) {
        std::cout << "ERROR: " << error_code << std::endl;
    }

    VnaDevice *vna = new KeysightM9807A("TCPIP0::K-N9020B-11111::inst0::INSTR");

    vna->full_preset();
    vna->init_channel();
    vna->set_power(-2.23f);
    vna->set_freq(11e9, 13e9, 113);

    return 0;
}
