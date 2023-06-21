#include <iostream>
#include "utils/logger.hpp"

#include "devices/visa_device.hpp"
#include "devices/device_set.hpp"


int main() {
    logger::set_log_level(LEVEL_DEBUG);

    logger::log(LEVEL_ERROR, "Error msg");
    logger::log(LEVEL_INFO, "Info msg");
    logger::log(LEVEL_DEBUG, "Debug msg");
    logger::log(LEVEL_WARN, "Warn msg");

    DeviceSet device_set{};
    device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::K-N9020B-11111::inst0::INSTR");

    return 0;
}
