#ifndef ANTESTL_BACKEND_TASK_MANAGER_HPP
#define ANTESTL_BACKEND_TASK_MANAGER_HPP

#include <json.hpp>
#include "devices/device_set.hpp"

#define WORD_TASK                   "task"
#define WORD_TASK_LIST              "task_list"

#define WORD_TASK_TYPE              "type"
#define WORD_TASK_ARGS              "args"

#define TASK_TYPE_CONNECT           "connect"
#define TASK_TYPE_CONFIGURE         "configure"
#define TASK_TYPE_SET_POWER         "set_power"
#define TASK_TYPE_SET_FREQ          "set_freq"
#define TASK_TYPE_SET_FREQ_RANGE    "set_freq_range"
#define TASK_TYPE_SET_ANGLE         "set_angle"
#define TASK_TYPE_SET_ANGLE_RANGE   "set_angle_range"
#define TASK_TYPE_CHANGE_PATH       "change_path"
#define TASK_TYPE_GET_DATA          "get_data"
#define TASK_TYPE_DISCONNECT        "disconnect"

#define DEVICE_EXT_GEN              "ext_gen"
#define DEVICE_RBD_UPKB             "rbd_upkb"
#define DEVICE_RBD_TESART           "rbd_tesart"

#define WORD_RESULT                 "result"
#define WORD_RESULT_ID              "id"
#define WORD_RESULT_MSG             "message"
#define WORD_RESULT_DATA            "data"

#define RESULT_OK_ID                0x00
#define RESULT_OK_MSG               "Complete"

#define VNA_NO_CONNECTION_ID        0x01
#define VNA_NO_CONNECTION_MSG       "No connection with vna"

#define EXT_GEN_NO_CONNECTION_ID    0x02
#define EXT_GEN_NO_CONNECTION_MSG   "No connection with external generator"

#define RBD_NO_CONNECTION_ID        0x03
#define RBD_NO_CONNECTION_MSG       "No connection with RBD"

#define VNA_CONFIGURE_ERR_ID        0x10
#define VNA_CONFIGURE_ERR_MSG       "Can't configure VNA"

#define ERR_SET_POWER_ID            0x20
#define ERR_SET_POWER_MSG           "Can't set power"

#define ERR_SET_FREQ_ID             0x30
#define ERR_SET_FREQ_MSG            "Can't set frequency"

#define ERR_SET_FREQ_RANGE_ID       0x31
#define ERR_SET_FREQ_RANGE_MSG      "Can't set frequency range"

#define ERR_SET_ANGLE_ID            0x40
#define ERR_SET_ANGLE_MSG           "Can't set angle"

#define ERR_SET_ANGLE_RANGE_ID      0x41
#define ERR_SET_ANGLE_RANGE_MSG     "Can't set angle range"

#define ERR_CHANGE_SWITCH_PATH_ID   0x50
#define ERR_CHANGE_SWITCH_PATH_MSG  "Can't change switch path"

#define ERR_GETTING_DATA_ID         0x60
#define ERR_GETTING_DATA_MSG        "Can't acquire data from VNA"

#define WRONG_TASK_TYPE_ID          0xFE
#define WRONG_TASK_TYPE_MSG         "Wrong task type"

#define NO_TASK_ID                  0xFF
#define NO_TASK_MSG                 "No task or task list"

using namespace nlohmann;

class TaskManager {
    DeviceSet device_set;

    json proceed_task(json task);
    json proceed_task_list(json task_list);

    json connect_task(json device_list);

    bool configure_task(json config_params);

    bool set_power_task(json power_value);

    bool set_freq_task(json freq_value);
    bool set_freq_range_task(json freq_range);

    bool set_angle_task(json angle_value);
    bool set_angle_range_task(json angle_range);

    bool change_path_task(json path_values);

    std::vector<std::string> get_data_task(json port_list);

public:
    TaskManager() = default;

    json parse_and_proceed(std::string input_data);
};


#endif //ANTESTL_BACKEND_TASK_MANAGER_HPP
