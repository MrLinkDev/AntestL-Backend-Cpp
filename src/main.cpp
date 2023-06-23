#include <iostream>
#include "utils/logger.hpp"

#include "devices/visa_device.hpp"
#include "devices/device_set.hpp"
#include "task_manager.hpp"


int main() {
    logger::set_log_level(LEVEL_TRACE);
    //logger::set_color_state(NO_COLOR);

    logger::log(LEVEL_ERROR, "Error msg");
    logger::log(LEVEL_INFO, "Info msg");
    logger::log(LEVEL_DEBUG, "Debug msg");
    logger::log(LEVEL_WARN, "Warn msg");
    logger::log(LEVEL_TRACE, "Trace msg");

    //DeviceSet device_set{};

    //device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::K-N9020B-11111::inst0::INSTR");
    //device_set.configure(MEAS_REFLECTION, 1e3, 1, false);

    //device_set.set_power(-14.7f);

    //device_set.set_freq_range(2e9, 7e9, 11);
    //device_set.get_data(new int[] {2, 5, 6}, 3, 0);

    std::string connection_task = "{\n"
                                  "\t\"task\": {\n"
                                  "\t\t\"type\": \"connect\",\n"
                                  "\t\t\"args\": {\n"
                                  "\t\t\t\"m9807a\": \"TCPIP0::K-N9020B-11111::inst0::INSTR\"\n"
                                  "\t\t}\n"
                                  "\t}}";

    std::string connection_task_2 = "{\n"
                                    "\t\"task\": {\n"
                                    "\t\t\"type\": \"connect\",\n"
                                    "\t\t\"args\": {\n"
                                    "\t\t\t\"m9807a\": \"TCPIP0::localhost::5025::SOCKET\",\n"
                                    "\t\t\t\"ext_gen\": \"TCPIP0::localhost::5026::SOCKET\",\n"
                                    "\t\t\t\"rbd_2\": \"TCPIP0::localhost::5001::SOCKET;TCPIP0::localhost::5002::SOCKET\"\n"
                                    "\t\t}\n"
                                    "\t}}";

    std::string configure_task = "{\n"
                                 "\t\"task\": {\n"
                                 "\t\t\"type\": \"configure\",\n"
                                 "\t\t\"args\": {\n"
                                 "\t\t\t\"meas_type\": 0,\n"
                                 "\t\t\t\"rbw\": 1e3,\n"
                                 "\t\t\t\"source_port\": 2,\n"
                                 "\t\t\t\"external\": false\n"
                                 "\t\t}\n"
                                 "\t}\n"
                                 "}";

    std::string set_power_task = "{\n"
                                 "\t\"task\": {\n"
                                 "\t\t\"type\": \"set_power\",\n"
                                 "\t\t\"args\": {\n"
                                 "\t\t\t\"value\": -13.2\n"
                                 "\t\t}\n"
                                 "\t}\n"
                                 "}";

    std::string set_freq_task = "{\n"
                                "\t\"task\": {\n"
                                "\t\t\"type\": \"set_freq\",\n"
                                "\t\t\"args\": {\n"
                                "\t\t\t\"value\": 3.7e9\n"
                                "\t\t}\n"
                                "\t}\n"
                                "}";

    std::string set_freq_range_task = "{\n"
                                      "\t\"task\": {\n"
                                      "\t\t\"type\": \"set_freq_range\",\n"
                                      "\t\t\"args\": {\n"
                                      "\t\t\t\"start\": 2e9,\n"
                                      "\t\t\t\"stop\": 5.8e9,\n"
                                      "\t\t\t\"points\": 11\n"
                                      "\t\t}\n"
                                      "\t}\n"
                                      "}";

    std::string change_path_task = "{\n"
                                   "\t\"task\": {\n"
                                   "\t\t\"type\": \"change_path\",\n"
                                   "\t\t\"args\": {\n"
                                   "\t\t\t\"switch_1\": 1,\n"
                                   "\t\t\t\"switch_2\": 5,\n"
                                   "\t\t\t\"switch_3\": 2,\n"
                                   "\t\t\t\"switch_4\": 4\n"
                                   "\t\t}\n"
                                   "\t}\n"
                                   "}";

    std::string get_data_task = "{\n"
                                "\t\"task\": {\n"
                                "\t\t\"type\": \"get_data\",\n"
                                "\t\t\"args\": {\n"
                                "\t\t\t\"ports\": [2, 6, 3]\n"
                                "\t\t}\n"
                                "\t}\n"
                                "}";

    TaskManager task_manager;
    std::string result;

    result = to_string(task_manager.parse_and_proceed(connection_task));
    logger::log(LEVEL_DEBUG, result);

    result = to_string(task_manager.parse_and_proceed(configure_task));
    logger::log(LEVEL_DEBUG, result);

    result = to_string(task_manager.parse_and_proceed(set_power_task));
    logger::log(LEVEL_DEBUG, result);

    result = to_string(task_manager.parse_and_proceed(set_freq_range_task));
    logger::log(LEVEL_DEBUG, result);

    result = to_string(task_manager.parse_and_proceed(change_path_task));
    logger::log(LEVEL_DEBUG, result);

    result = to_string(task_manager.parse_and_proceed(get_data_task));
    logger::log(LEVEL_DEBUG, result);

    return 0;
}
