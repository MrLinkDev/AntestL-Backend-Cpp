#include "task_manager.hpp"

json TaskManager::proceed_task(json task) {
    json result;
    logger::log(LEVEL_TRACE, "\'proceed_task\' got argument = {}", to_string(task));

    if (task[WORD_TASK_TYPE] == TASK_TYPE_CONNECT) {
        result[WORD_RESULT] = connect_task(task[WORD_TASK_ARGS]);
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_CONFIGURE) {
        bool task_result = configure_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, task_result ? RESULT_OK_ID : VNA_CONFIGURE_ERR_ID},
                {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : VNA_CONFIGURE_ERR_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_POWER) {
        bool task_result = set_power_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
            {WORD_RESULT_ID, task_result ? RESULT_OK_ID : ERR_SET_POWER_ID},
            {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : ERR_SET_POWER_MSG},
            {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_FREQ) {
        bool task_result = set_freq_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, task_result ? RESULT_OK_ID : ERR_SET_FREQ_ID},
                {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : ERR_SET_FREQ_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_FREQ_RANGE) {
        bool task_result = set_freq_range_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, task_result ? RESULT_OK_ID : ERR_SET_FREQ_RANGE_ID},
                {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : ERR_SET_FREQ_RANGE_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_ANGLE) {
        bool task_result = set_angle_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, task_result ? RESULT_OK_ID : ERR_SET_ANGLE_ID},
                {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : ERR_SET_ANGLE_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_SET_ANGLE_RANGE) {
        bool task_result = set_angle_range_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, task_result ? RESULT_OK_ID : ERR_SET_ANGLE_RANGE_ID},
                {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : ERR_SET_ANGLE_RANGE_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_CHANGE_PATH) {
        bool task_result = change_path_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, task_result ? RESULT_OK_ID : ERR_CHANGE_SWITCH_PATH_ID},
                {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : ERR_CHANGE_SWITCH_PATH_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_GET_DATA) {
        std::vector<std::string> task_result = get_data_task(task[WORD_TASK_ARGS]);
        std::string angle_list = device_set.get_current_angle_list();

        std::string data{};

        if (!task_result.empty()) {
            for (int point_pos = 0; point_pos < task_result.size(); ++point_pos) {
                if (point_pos == 0) {
                    if (!angle_list.empty()) {
                        data = angle_list + COLUMN_DELIMITER;
                    }

                    data += std::to_string(device_set.get_current_freq(point_pos)) + COLUMN_DELIMITER + task_result[point_pos];
                } else {
                    data += ROW_DELIMITER;

                    if (!angle_list.empty()) {
                        data += angle_list + COLUMN_DELIMITER;
                    }

                    data += std::to_string(device_set.get_current_freq(point_pos)) + COLUMN_DELIMITER + task_result[point_pos];
                }
            }
        }

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, !task_result.empty() ? RESULT_OK_ID : ERR_GETTING_DATA_ID},
                {WORD_RESULT_MSG, !task_result.empty() ? RESULT_OK_MSG : ERR_GETTING_DATA_MSG},
                {WORD_RESULT_DATA, data}
        };
    } else {
        result[WORD_RESULT] = {
                {WORD_RESULT_ID, WRONG_TASK_TYPE_ID},
                {WORD_RESULT_MSG, WRONG_TASK_TYPE_MSG},
                {WORD_RESULT_DATA, false}
        };
    }

    logger::log(
            result[WORD_RESULT][WORD_RESULT_ID] == 0 ? LEVEL_INFO : LEVEL_ERROR,
            result[WORD_RESULT][WORD_RESULT_ID] == 0 ? "Task completed" : "Can't proceed task. Error code: {}",
            to_string(result[WORD_RESULT][WORD_RESULT_ID]));

    return result;
}

json TaskManager::proceed_task_list(json task_list) {
    json result;
    json nested_result;

    std::vector<json> nested_task_list{};
    std::vector<json> task_cache = task_list.get<std::vector<json>>();

    for (json task : task_cache) {
        logger::log(LEVEL_DEBUG, to_string(task));
        if (!task.contains(WORD_NESTED)) {
            result = proceed_task(task);

            if (result[WORD_RESULT][WORD_RESULT_ID] != 0) {
                return result;
            }
        } else if (task.contains(WORD_NESTED) && (task[WORD_TASK_TYPE] == TASK_TYPE_CONNECT ||
        task[WORD_TASK_TYPE] == TASK_TYPE_CONFIGURE || task[WORD_TASK_TYPE] == TASK_TYPE_SET_POWER ||
        task[WORD_TASK_TYPE] == TASK_TYPE_SET_ANGLE || task[WORD_TASK_TYPE] == TASK_TYPE_SET_FREQ ||
        task[WORD_TASK_TYPE] == TASK_TYPE_DISCONNECT)) {
            logger::log(
                    LEVEL_WARN,
                    R"(Task '{}' has arg 'nested', but this task cannot be nested. This arg ignored.)",
                    task[WORD_TASK_TYPE].get<std::string>());

            task.erase(WORD_NESTED);
            result = proceed_task(task);

            if (result[WORD_RESULT][WORD_RESULT_ID] != 0) {
                return result;
            }
        } else {
            nested_task_list.push_back(task);
        }
    }

    if (result[WORD_RESULT][WORD_RESULT_ID] != 0) {
        return result;
    }

    std::sort(nested_task_list.begin(), nested_task_list.end(), array_utils::compare_nested);
    nested_result = proceed_nested_task_list(nested_task_list);

    if (nested_result[WORD_RESULT][WORD_RESULT_ID] != 0) {
        return nested_result;
    } else {
        result[WORD_RESULT][WORD_RESULT_DATA] = nested_result[WORD_RESULT][WORD_RESULT_DATA];
    }

    return result;
}

json TaskManager::proceed_nested_task_list(std::vector<json> nested_task_list) {
    json result;
    std::string data{};

    for (json &nested_task : nested_task_list) {
        if (nested_task[WORD_TASK_TYPE] == TASK_TYPE_GET_DATA) {
            continue;
        } else if (nested_task[WORD_TASK_TYPE] == TASK_TYPE_SET_ANGLE_RANGE) {
            logger::log(LEVEL_DEBUG, to_string(nested_task[WORD_TASK_ARGS]));
            result = proceed_task(nested_task);

            if (result[WORD_RESULT][WORD_RESULT_ID] != 0) {
                return result;
            }

            nested_task[WORD_TASK_TYPE] = TASK_TYPE_NEXT_ANGLE;
            nested_task[WORD_AXIS] = nested_task[WORD_TASK_ARGS][WORD_AXIS];

            nested_task.erase(WORD_TASK_ARGS);
        } else if (nested_task[WORD_TASK_TYPE] == TASK_TYPE_SET_FREQ_RANGE) {
            result = proceed_task(nested_task);

            if (result[WORD_RESULT][WORD_RESULT_ID] != 0) {
                return result;
            }

            nested_task[WORD_TASK_TYPE] = TASK_TYPE_NEXT_FREQ;
            nested_task.erase(WORD_TASK_ARGS);
        }
    }

    for (int nested_pos = 0; nested_pos < nested_task_list.size(); ++nested_pos) {
        logger::log(LEVEL_DEBUG, "Nested pos = {}", nested_pos);

        if (nested_task_list[nested_pos][WORD_TASK_TYPE] == TASK_TYPE_GET_DATA) {
            std::vector<std::string> task_result = get_data_task(nested_task_list[nested_pos][WORD_TASK_ARGS]);
            std::string angle_list = device_set.get_current_angle_list();

            if (!task_result.empty()) {
                for (int point_pos = 0; point_pos < task_result.size(); ++point_pos) {
                    if (!data.empty()) {
                        data += ROW_DELIMITER;
                    }

                    if (!angle_list.empty()) {
                        data += angle_list + COLUMN_DELIMITER;
                    }

                    data += std::to_string(device_set.get_current_freq(point_pos)) + COLUMN_DELIMITER + task_result[point_pos];
                }
            } else {
                result[WORD_RESULT] = {
                        {WORD_RESULT_ID, ERR_GETTING_DATA_ID},
                        {WORD_RESULT_MSG, ERR_GETTING_DATA_MSG},
                        {WORD_RESULT_DATA, data}
                };

                return result;
            }
        } else if (nested_task_list[nested_pos][WORD_TASK_TYPE] == TASK_TYPE_NEXT_FREQ) {
            if (!device_set.is_using_ext_gen()) {
                continue;
            } else {
                switch (next_freq_task()) {
                    case NEXT_FREQ_OK:
                        nested_pos = -1;
                        continue;
                    case NEXT_FREQ_BOUND:
                        if (device_set.move_to_start_freq()) {
                            continue;
                        } else {
                            result[WORD_RESULT] = {
                                    {WORD_RESULT_ID, ERR_SET_FREQ_ID},
                                    {WORD_RESULT_MSG, ERR_SET_FREQ_MSG},
                                    {WORD_RESULT_DATA, false}
                            };
                        }
                    default:
                        result[WORD_RESULT] = {
                                {WORD_RESULT_ID, ERR_SET_FREQ_ID},
                                {WORD_RESULT_MSG, ERR_SET_FREQ_MSG},
                                {WORD_RESULT_DATA, false}
                        };

                        return result;
                }
            }
        } else if (nested_task_list[nested_pos][WORD_TASK_TYPE] == TASK_TYPE_NEXT_ANGLE) {
            switch (next_angle_task(nested_task_list[nested_pos][WORD_AXIS].get<int>())) {
                case NEXT_ANGLE_OK:
                    nested_pos = -1;
                    continue;
                case NEXT_ANGLE_BOUND:
                    if (device_set.move_to_start_angle(nested_task_list[nested_pos][WORD_AXIS].get<int>())) {
                        continue;
                    } else {
                        result[WORD_RESULT] = {
                                {WORD_RESULT_ID, ERR_SET_ANGLE_ID},
                                {WORD_RESULT_MSG, ERR_SET_ANGLE_MSG},
                                {WORD_RESULT_DATA, false}
                        };
                    }
                default:
                    result[WORD_RESULT] = {
                            {WORD_RESULT_ID, ERR_SET_ANGLE_ID},
                            {WORD_RESULT_MSG, ERR_SET_ANGLE_MSG},
                            {WORD_RESULT_DATA, false}
                    };

                    return result;
            }
        }
    }

    result[WORD_RESULT] = {
            {WORD_RESULT_ID, RESULT_OK_ID},
            {WORD_RESULT_MSG, RESULT_OK_MSG},
            {WORD_RESULT_DATA, data}
    };

    return result;
}

json TaskManager::connect_task(nlohmann::json device_list) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_CONNECT);

    json output = {
            {WORD_RESULT_ID, RESULT_OK_ID},
            {WORD_RESULT_MSG, RESULT_OK_MSG}
    };
    json device_results;

    for (const auto &json_item : device_list.items()) {
        std::string device = json_item.key();

        if (device == DEVICE_EXT_GEN) {
            device_results[json_item.key()] = device_set.connect(DEVICE_GEN, device, json_item.value());

            if (!device_results[json_item.key()]) {
                output[WORD_RESULT_ID] = EXT_GEN_NO_CONNECTION_ID;
                output[WORD_RESULT_MSG] = EXT_GEN_NO_CONNECTION_MSG;
                break;
            }
        } else if (device == DEVICE_RBD_UPKB || device == DEVICE_RBD_TESART || device == DEVICE_RBD_DEMO) {
            device_results[json_item.key()] = device_set.connect(DEVICE_RBD, device, json_item.value());

            if (!device_results[json_item.key()]) {
                output[WORD_RESULT_ID] = RBD_NO_CONNECTION_ID;
                output[WORD_RESULT_MSG] = RBD_NO_CONNECTION_MSG;
                break;
            }
        } else {
            device_results[json_item.key()] = device_set.connect(DEVICE_VNA, device, json_item.value());

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

bool TaskManager::configure_task(json config_params) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_CONFIGURE);

    int meas_type   = config_params["meas_type"].get<int>();
    float rbw       = config_params["rbw"].get<float>();
    int source_port = config_params["source_port"].get<int>();
    bool external   = config_params["external"].get<bool>();

    bool result = device_set.configure(meas_type, rbw, source_port, external);
    return result;
}

bool TaskManager::set_power_task(json power_value) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_SET_POWER);

    float value = power_value["value"].get<float>();

    bool result = device_set.set_power(value);
    return result;
}

bool TaskManager::set_freq_task(json freq_value) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_SET_FREQ);

    double value = freq_value["value"].get<double>();

    bool result = device_set.set_freq(value);
    return result;
}

bool TaskManager::set_freq_range_task(json freq_range) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_SET_FREQ_RANGE);

    double start_freq   = freq_range["start"].get<double>();
    double stop_freq    = freq_range["stop"].get<double>();
    int points          = freq_range["points"].get<int>();

    bool result = device_set.set_freq_range(start_freq, stop_freq, points);
    result &= device_set.move_to_start_freq();

    return result;
}

bool TaskManager::set_angle_task(json angle_value) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_SET_ANGLE);

    double value = angle_value["value"].get<double>();
    int axis_num = angle_value["axis"].get<int>();

    bool result = device_set.set_angle(value, axis_num);
    return result;
}

bool TaskManager::set_angle_range_task(json angle_range) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_SET_ANGLE_RANGE);

    double start_angle  = angle_range["start"].get<double>();
    double stop_angle   = angle_range["stop"].get<double>();
    int points          = angle_range["points"].get<int>();
    int axis_num        = angle_range["axis"].get<int>();

    bool result = device_set.set_angle_range(start_angle, stop_angle, points, axis_num);
    result &= device_set.move_to_start_angle(axis_num);

    return result;
}

bool TaskManager::change_path_task(json path_values) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_CHANGE_PATH);

    std::vector<int> paths{};

    for (int switch_module_num = 1; switch_module_num < device_set.get_vna_switch_module_count() + 1; ++switch_module_num) {
        if (path_values.contains(std::format("switch_{}", switch_module_num))) {
            paths.push_back(path_values[std::format("switch_{}", switch_module_num)].get<int>());
        } else {
            paths.push_back(-1);
        }
    }

    bool result = device_set.change_path(paths);
    return result;
}

std::vector<std::string> TaskManager::get_data_task(json port_list) {
    logger::log(LEVEL_INFO, "Received \'{}\' task", TASK_TYPE_GET_DATA);

    std::vector<int> ports = port_list["ports"].get<std::vector<int>>();
    std::vector<iq_data_t> port_data = device_set.get_data(ports);

    std::vector<std::string> result_data{};

    for (int point_pos = 0; point_pos < port_data[0].size(); ++point_pos) {
        std::string point_data{};

        for (int port_pos = 0; port_pos < port_data.size(); ++port_pos) {
            if (port_pos == 0) {
                point_data = port_data[port_pos][point_pos].first + COLUMN_DELIMITER + port_data[port_pos][point_pos].second;
            } else {
                point_data += COLUMN_DELIMITER + port_data[port_pos][point_pos].first + COLUMN_DELIMITER + port_data[port_pos][point_pos].second;
            }
        }

        result_data.push_back(point_data);
    }

    return result_data;
}

int TaskManager::next_freq_task() {
    return device_set.next_freq();
}

int TaskManager::next_angle_task(int axis_num) {
    return device_set.next_angle(axis_num);
}

json TaskManager::parse_and_proceed(std::string input_data) {
    json data = json::parse(input_data);
    json answer;

    logger::log(LEVEL_DEBUG, "Input data: {}", to_string(data));

    if (data.contains(WORD_TASK)) {
        logger::log(LEVEL_INFO, "Received task");
        answer = proceed_task(data[WORD_TASK]);
    } else if (data.contains(WORD_TASK_LIST)) {
        logger::log(LEVEL_INFO, "Received task list");
        answer = proceed_task_list(data[WORD_TASK_LIST]);
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


