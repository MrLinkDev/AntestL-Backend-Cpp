#ifndef ANTESTL_BACKEND_TASK_MANAGER_HPP
#define ANTESTL_BACKEND_TASK_MANAGER_HPP

#include "device_set.hpp"
#include <json.hpp>
#include <iostream>

#define WORD_TASK                   "task"
#define WORD_TASK_LIST              "task_list"

#define WORD_TASK_TYPE              "type"
#define WORD_TASK_ARGS              "args"

#define TASK_TYPE_CONNECT           "connect"
#define TASK_TYPE_CONFIGURE         "configure"
#define TASK_TYPE_SET_POWER         "set_power"
#define TASK_TYPE_SET_FREQ          "set_freq_range"
#define TASK_TYPE_SET_ANGLE         "set_angle_range"
#define TASK_TYPE_CHANGE_PATH       "change_path"
#define TASK_TYPE_GET_DATA          "get_data"

#define DEVICE_EXT_GEN              "ext_gen"
#define DEVICE_RBD_1                "rbd_1"
#define DEVICE_RBD_2                "rbd_2"

#define WORD_RESULT                 "result"
#define WORD_RESULT_ID              "id"
#define WORD_RESULT_MSG             "message"
#define WORD_RESULT_DATA            "data"

#define RESULT_OK_ID                0x00
#define RESULT_OK_MSG               "OK"

#define VNA_CONFIGURE_ERR_ID        0x01
#define VNA_CONFIGURE_ERR_MSG       "Can't configure VNA"

#define VNA_NO_CONNECTION_ID        0x02
#define VNA_NO_CONNECTION_MSG       "No connection with vna"

#define EXT_GEN_NO_CONNECTION_ID    0x03
#define EXT_GEN_NO_CONNECTION_MSG   "No connection with external generator"

#define RBD_NO_CONNECTION_ID        0x04
#define RBD_NO_CONNECTION_MSG       "No connection with RBD"

#define ERR_SET_POWER_ID            0x05
#define ERR_SET_POWER_MSG           "Can't set power"

#define ERR_SET_FREQ_RANGE_ID       0x06
#define ERR_SET_FREQ_RANGE_MSG      "Can't set frequency range"

#define ERR_SET_ANGLE_RANGE_ID      0x07
#define ERR_SET_ANGLE_RANGE_MSG     "Can't set angle range"

#define ERR_CHANGE_SWITCH_PATH_ID   0x08
#define ERR_CHANGE_SWITCH_PATH_MSG  "Can't change switch path"

#define ERR_GETTING_DATA_ID         0x09
#define ERR_GETTING_DATA_MSG        "Can't acquire data from VNA"

#define WRONG_TASK_TYPE_ID          0xFE
#define WRONG_TASK_TYPE_MSG         "Wrong task type"

#define NO_TASK_ID                  0xFF
#define NO_TASK_MSG                 "No task or task list"

using namespace nlohmann;
using namespace std;

class TaskManager {
    DeviceSet device_set;

    json proceed_task(json task) {
        json result;

        if (task[WORD_TASK_TYPE] == TASK_TYPE_CONNECT) {
            result[WORD_RESULT] = connect(task[WORD_TASK_ARGS]);
        } else if (task[WORD_TASK_TYPE] == TASK_TYPE_CONFIGURE) {
            result[WORD_RESULT] = configure(task[WORD_TASK_ARGS]);
        } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_POWER) {
            result[WORD_RESULT] = set_power(task[WORD_TASK_ARGS]);
        } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_FREQ) {
            result[WORD_RESULT] = set_freq(task[WORD_TASK_ARGS]);
        } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_ANGLE) {
            result[WORD_RESULT] = set_angle(task[WORD_TASK_ARGS]);
        } else if (task[WORD_TASK_TYPE] == TASK_TYPE_CHANGE_PATH) {
            result[WORD_RESULT] = change_path(task[WORD_TASK_ARGS]);
        } else if (task[WORD_TASK_TYPE] == TASK_TYPE_GET_DATA) {
            result[WORD_RESULT] = get_data(task[WORD_TASK_ARGS]);
        } else {
            result[WORD_RESULT] = {
                {WORD_RESULT_ID, WRONG_TASK_TYPE_ID},
                {WORD_RESULT_MSG, WRONG_TASK_TYPE_MSG},
                {WORD_RESULT_DATA, false}
            };
        }

        string message = "Task completed with result: ";
        message.append(result[WORD_RESULT][WORD_RESULT_MSG]);

        if (result[WORD_RESULT][WORD_RESULT_ID] == 0) {
            logger::log(LEVEL_INFO, message.c_str(), NULL);
        } else {
            logger::log(LEVEL_ERROR, message.c_str(), NULL);
        }

        return result;
    }

    json proceed_task_list(json task_list) {
        return json();
    }

    json connect(json device_list) {
        logger::log(LEVEL_INFO, "Got connection task", NULL);

        json output = {
                {WORD_RESULT_ID, RESULT_OK_ID},
                {WORD_RESULT_MSG, RESULT_OK_MSG}
        };
        json device_results;

        for (const auto& json_item : device_list.items()) {
            string device = json_item.key();

            if (device == DEVICE_EXT_GEN) {

            } else if (device == DEVICE_RBD_1) {

            } else if (device == DEVICE_RBD_2) {

            } else {
                device_results[json_item.key()] = device_set.connect_to_vna(device, json_item.value());

                if (!device_results[json_item.key()]) {
                    output[WORD_RESULT_ID] = VNA_NO_CONNECTION_ID;
                    output[WORD_RESULT_MSG] = VNA_NO_CONNECTION_MSG;
                    break;
                }
            }
        }

        output.push_back({WORD_RESULT_DATA, device_results});
        return output;
    }

    json configure(json params) {
        logger::log(LEVEL_INFO, "Configuring VNA", NULL);

        bool result = device_set.configure(params);

        json output = {
                {WORD_RESULT_ID, result ? RESULT_OK_ID : VNA_CONFIGURE_ERR_ID},
                {WORD_RESULT_MSG, result ? RESULT_OK_MSG : VNA_CONFIGURE_ERR_MSG},
                {WORD_RESULT_DATA, result}
        };

        return output;
    }

    json set_power(json params) {
        logger::log(LEVEL_INFO, "Setting power", NULL);

        bool result = device_set.set_power(params);

        json output = {
                {WORD_RESULT_ID, result ? RESULT_OK_ID : ERR_SET_POWER_ID},
                {WORD_RESULT_MSG, result ? RESULT_OK_MSG : ERR_SET_POWER_MSG},
                {WORD_RESULT_DATA, result}
        };

        return output;
    }

    json set_freq(json params) {
        logger::log(LEVEL_INFO, "Setting frequency range", NULL);

        bool result = device_set.set_freq(params);

        json output = {
                {WORD_RESULT_ID, result ? RESULT_OK_ID : ERR_SET_FREQ_RANGE_ID},
                {WORD_RESULT_MSG, result ? RESULT_OK_MSG : ERR_SET_FREQ_RANGE_MSG},
                {WORD_RESULT_DATA, result}
        };

        return output;
    }

    json set_angle(json params) {
        logger::log(LEVEL_INFO, "Setting angle range", NULL);

        bool result = device_set.set_angle(params);

        json output = {
                {WORD_RESULT_ID, result ? RESULT_OK_ID : ERR_SET_ANGLE_RANGE_ID},
                {WORD_RESULT_MSG, result ? RESULT_OK_MSG : ERR_SET_ANGLE_RANGE_MSG},
                {WORD_RESULT_DATA, result}
        };

        return output;
    }

    json change_path(json params) {
        logger::log(LEVEL_INFO, "Changing switch paths", NULL);

        bool result = device_set.change_path(params);

        json output = {
                {WORD_RESULT_ID, result ? RESULT_OK_ID : ERR_CHANGE_SWITCH_PATH_ID},
                {WORD_RESULT_MSG, result ? RESULT_OK_MSG : ERR_CHANGE_SWITCH_PATH_MSG},
                {WORD_RESULT_DATA, result}
        };

        return output;
    }

    json get_data(json params) {
        logger::log(LEVEL_INFO, "Acquiring data from VNA", NULL);

        json result = device_set.get_data(params);

        json output = {
                {WORD_RESULT_ID, result ? RESULT_OK_ID : ERR_GETTING_DATA_ID},
                {WORD_RESULT_MSG, result ? RESULT_OK_MSG : ERR_GETTING_DATA_MSG},
                {WORD_RESULT_DATA, result}
        };

        return output;
    }

public:
    TaskManager() = default;

    json parse_data(string *input_data) {
        json data = json::parse(*input_data);
        json answer;

        if (data.contains(WORD_TASK)) {
            answer = proceed_task(data[WORD_TASK]);
        } else if (data.contains(WORD_TASK_LIST)) {
            answer = proceed_task_list(data);
        } else {
            answer = {
                    WORD_RESULT, {
                        {WORD_RESULT_ID, NO_TASK_ID},
                        {WORD_RESULT_MSG, NO_TASK_MSG}
                    }
            };
        }

        return answer;
    }
};

#endif //ANTESTL_BACKEND_TASK_MANAGER_HPP
