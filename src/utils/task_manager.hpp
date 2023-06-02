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

#define DEVICE_EXT_GEN              "ext_gen"
#define DEVICE_RBD_1                "rbd_1"
#define DEVICE_RBD_2                "rbd_2"

#define WORD_RESULT                 "result"
#define WORD_RESULT_ID              "id"
#define WORD_RESULT_MSG             "message"
#define WORD_RESULT_DATA            "data"

#define RESULT_OK_ID                0x00
#define RESULT_OK_MSG               "OK"

#define VNA_NO_CONNECTION_ID        0x01
#define VNA_NO_CONNECTION_MSG       "No connection with vna"

#define EXT_GEN_NO_CONNECTION_ID    0x02
#define EXT_GEN_NO_CONNECTION_MSG   "No connection with external generator"

#define RBD_NO_CONNECTION_ID        0x03
#define RBD_NO_CONNECTION_MSG       "No connection with RBD"

#define WRONG_TASK_ID               0xFF
#define WRONG_TASK_MSG              "No task or task list"

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
        logger::log(LEVEL_INFO, "Got configure task", NULL);

        device_set.configure(params);

        json output = {
                {WORD_RESULT_ID, RESULT_OK_ID},
                {WORD_RESULT_MSG, RESULT_OK_MSG},
                {WORD_RESULT_DATA, true}
        };

        return output;
    }

    json set_power(json params) {

    }

    json set_freq(json params) {

    }

    json

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
                        {WORD_RESULT_ID, WRONG_TASK_ID},
                        {WORD_RESULT_MSG, WRONG_TASK_MSG}
                    }
            };
        }

        return answer;
    }
};

#endif //ANTESTL_BACKEND_TASK_MANAGER_HPP
