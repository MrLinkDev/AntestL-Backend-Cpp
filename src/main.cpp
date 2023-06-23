#include <iostream>
#include "utils/logger.hpp"

#include "devices/visa_device.hpp"
#include "devices/device_set.hpp"


int main() {
    logger::set_log_level(LEVEL_DEBUG);
    //logger::set_color_state(NO_COLOR);

    logger::log(LEVEL_ERROR, "Error msg");
    logger::log(LEVEL_INFO, "Info msg");
    logger::log(LEVEL_DEBUG, "Debug msg");
    logger::log(LEVEL_WARN, "Warn msg");

    DeviceSet device_set{};

    device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::K-N9020B-11111::inst0::INSTR");
    device_set.configure(MEAS_REFLECTION, 1e3, 1, false);

    device_set.set_power(-14.7f);

    device_set.set_freq_range(2e9, 7e9, 11);
    device_set.get_data(new int[] {2, 5, 6}, 3, 0);

    return 0;
}
