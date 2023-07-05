/**
 * \file
 * \brief Файл исходного кода, в котором реализованы методы для класа TaskManager
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#include "task_manager.hpp"
#include "utils/array_utils.hpp"

/**
 * \brief Метод, обрабатывающий задание на подключение
 *
 * \param [in] device_list JSON объект, содержащий в себе список устройств и адресов
 * этих устройств
 *
 * \return Результат выполнения задания
 */
json TaskManager::connect_task(nlohmann::json device_list) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_CONNECT);

    json output = {
            {WORD_RESULT_ID, RESULT_OK_ID},
            {WORD_RESULT_MSG, RESULT_OK_MSG}
    };
    json device_results;

    for (const auto &json_item : device_list.items()) {
        const std::string &device = json_item.key();

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

/**
 * \brief Метод, обрабатывающий задание на отключение
 */
void TaskManager::disconnect_task() {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_DISCONNECT);

    device_set.disconnect();
    logger::log(LEVEL_DEBUG, "Disconnected from devices");
}

/**
 * \brief Метод, обрабатывающий задание на настройку ВАЦ
 *
 * \param [in] config_params JSON объект, в котором хранится список параметров
 * для настройки ВАЦ
 *
 * \return Если задание обработано успешно - true. В противном случае - false.
 */
bool TaskManager::configure_task(json config_params) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_CONFIGURE);

    int meas_type = config_params["meas_type"].get<int>();
    float rbw = config_params["rbw"].get<float>();
    int source_port = config_params["source_port"].get<int>();
    bool external = config_params["external"].get<bool>();
    
    logger::log(
            LEVEL_DEBUG, 
            R"(Configuring VNA with parameters: "meas_type" = {}; "rbw" = {}; "source_port" = {}; "external" = {})",
            meas_type, rbw, source_port, external);

    bool result = device_set.configure(meas_type, rbw, source_port, external);
    return result;
}

/**
 * \brief Метод, обрабатывающий задание на изменение мощности
 *
 * \param [in] power_value JSON объект, в котором хранится значение мощности
 *
 * \return Если задание обработано успешно - true. В противном случае - false.
 */
bool TaskManager::set_power_task(json power_value) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_SET_POWER);

    float value = power_value["value"].get<float>();

    bool result = device_set.set_power(value);
    return result;
}

/**
 * \brief Метод, обрабатывающий задание на изменение частоты
 *
 * \param freq_value JSON объект, в котором хранится значение частоты
 *
 * \return Если задание обработано успешно - true. В противном случае - false.
 */
bool TaskManager::set_freq_task(json freq_value) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_SET_FREQ);

    double value = freq_value["value"].get<double>();

    bool result = device_set.set_freq(value);
    return result;
}

/**
 * \brief Метод, обрабатывающий задание на изменение частотного диапазона
 *
 * \param freq_range JSON объект, в котором хранятся данные о частотном диапазоне
 *
 * \return Если задание обработано успешно - true. В противном случае - false.
 */
bool TaskManager::set_freq_range_task(json freq_range) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_SET_FREQ_RANGE);

    double start_freq = freq_range["start"].get<double>();
    double stop_freq = freq_range["stop"].get<double>();
    int points = freq_range["points"].get<int>();
    
    logger::log(
            LEVEL_DEBUG, 
            R"(Frequency range: "start_freq" = {}; "stop_freq" = {}; "points" = {})",
            start_freq, stop_freq, points);

    bool result = device_set.set_freq_range(start_freq, stop_freq, points);
    result &= device_set.move_to_start_freq();

    return result;
}

/**
 * \brief Метод, обрабатывающий задание на сдвиг частотной точки
 *
 * \return Если следующая частотная точка находится в пределах
 * границ изменения, то возвращает FREQ_MOVE_OK. Если точка
 * находится на границе, то возвращает FREQ_MOVE_BOUND.
 */
int TaskManager::next_freq_task() {
    return device_set.next_freq();
}

/**
 * \brief Метод, обрабатывающий задание на изменение угла
 *
 * \param angle_value JSON объект, в котором хранится значение угла
 *
 * \return Если задание обработано успешно - true. В противном случае - false.
 */
bool TaskManager::set_angle_task(json angle_value) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_SET_ANGLE);

    float value = angle_value["value"].get<float>();
    int axis_num = angle_value["axis"].get<int>();
    
    logger::log(LEVEL_DEBUG, "Axis {}: angle = {}", axis_num, value);

    bool result = device_set.set_angle(value, axis_num);
    return result;
}

/**
 * \brief Метод, обрабатывающий задание на изменение углового диапазона
 *
 * \param angle_range JSON объект, в котором хранятся данные об угловом диапазоне
 *
 * \return Если задание обработано успешно - true. В противном случае - false.
 */
bool TaskManager::set_angle_range_task(json angle_range) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_SET_ANGLE_RANGE);

    float start_angle = angle_range["start"].get<float>();
    float stop_angle = angle_range["stop"].get<float>();
    int points = angle_range["points"].get<int>();
    int axis_num = angle_range["axis"].get<int>();

    logger::log(
            LEVEL_DEBUG,
            R"(Angle range for axis {}: "start" = {}; "stop" = {}; "points" = {})",
            axis_num, start_angle, stop_angle, points);

    bool result = device_set.set_angle_range(start_angle, stop_angle, points, axis_num);
    result &= device_set.move_to_start_angle(axis_num);

    return result;
}

/**
 * \brief Метод, обрабатывающий задание на сдвиг угловой точки
 *
 * \param [in] axis_num Номер оси ОПУ
 *
 * \return Если угловая точка находится в пределах диапазона изменения угла,
 * то возвращается ANGLE_MOVE_OK. Если угловая точка находится на границе
 * диапазона, то возвращает ANGLE_MOVE_BOUND.
 */
int TaskManager::next_angle_task(int axis_num) {
    return device_set.next_angle(axis_num);
}

/**
 * \brief Метод, обрабатывающий задание на изменение положений переключателей
 *
 * \param [in] path_values JSON объект, который содержит список требуемых положений
 * переключателей
 *
 * \return Если действие выполнено успешно, возвращает true. В противном
 * случае - false.
 */
bool TaskManager::set_path_task(json path_values) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_CHANGE_PATH);

    std::vector<int> paths{};

    for (int switch_module_num = 1; switch_module_num < device_set.get_vna_switch_module_count() + 1; ++switch_module_num) {
        if (path_values.contains(std::format("switch_{}", switch_module_num))) {
            paths.push_back(path_values[std::format("switch_{}", switch_module_num)].get<int>());
        } else {
            paths.push_back(-1);
        }
    }

    std::string data = "Paths: ";

    for (int i = 1; i < paths.size() + 1; ++i) {
        data += std::format("\"switch_{}\" = {}{}", i, paths[i], (i == paths.size() ? "" : ";"));
    }

    logger::log(LEVEL_DEBUG, data);

    bool result = device_set.set_path(std::move(paths));
    return result;
}

/**
 * \brief Метод, обрабатывающий задание на проведение измерения и сбор данных
 *
 * \param [in] port_list JSON объект, который содержит список портов ВАЦ, для которых
 * требуется провести измерение
 *
 * \return Если действие выполнено успешно, возвращает полученные данные. В противном
 * случае, возвращает пустую строку.
 */
std::string TaskManager::get_data_task(json port_list) {
    logger::log(LEVEL_TRACE, "Received \"{}\" task", TASK_TYPE_GET_DATA);

    std::vector<int> ports = port_list["ports"].get<std::vector<int>>();

    data_t acquired_data = device_set.get_data(ports);
    return acquired_data.to_string();
}

/**
 * \brief Метод, обрабатывающий пришедшее задание.
 *
 * Определяется тип задания, после чего вызывается соответствующий метод для обработки
 * задания. После обработки - формируется JSON объект с результатами выполнения.
 *
 * \param [in] task Задание, которое требуется обработать
 *
 * \return Результат обработки задания
 */
json TaskManager::proceed_task(const json &task) {
    json result;

    if (task[WORD_TASK_TYPE] == TASK_TYPE_CONNECT) {
        result[WORD_RESULT] = connect_task(task[WORD_TASK_ARGS]);
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_DISCONNECT) {
        disconnect_task();

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, RESULT_OK_ID},
                {WORD_RESULT_MSG, RESULT_OK_MSG},
                {WORD_RESULT_DATA, true}
        };
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
        bool task_result = set_path_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, task_result ? RESULT_OK_ID : ERR_CHANGE_SWITCH_PATH_ID},
                {WORD_RESULT_MSG, task_result ? RESULT_OK_MSG : ERR_CHANGE_SWITCH_PATH_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else if (task[WORD_TASK_TYPE] == TASK_TYPE_GET_DATA) {
        std::string task_result = get_data_task(task[WORD_TASK_ARGS]);

        result[WORD_RESULT] = {
                {WORD_RESULT_ID, !task_result.empty() ? RESULT_OK_ID : ERR_GETTING_DATA_ID},
                {WORD_RESULT_MSG, !task_result.empty() ? RESULT_OK_MSG : ERR_GETTING_DATA_MSG},
                {WORD_RESULT_DATA, task_result}
        };
    } else {
        result[WORD_RESULT] = {
                {WORD_RESULT_ID, WRONG_TASK_TYPE_ID},
                {WORD_RESULT_MSG, WRONG_TASK_TYPE_MSG},
                {WORD_RESULT_DATA, false}
        };
    }

    logger::log(
            result[WORD_RESULT][WORD_RESULT_ID] == 0 ? LEVEL_DEBUG : LEVEL_ERROR,
            result[WORD_RESULT][WORD_RESULT_ID] == 0 ? "Task completed" : "Can't proceed task. Error code: {}",
            to_string(result[WORD_RESULT][WORD_RESULT_ID]));

    return result;
}

/**
 * \brief Метод, обрабатывающий список заданий.
 *
 * Выделяются задания, для которых требуется обработка вложенности. Полученный
 * список заданий сортируется по уровню вложенности и передаётся в метод
 * proceed_nested_task_list() для дальнейшей обработки. Затем, формируется
 * результат обработки всех заданий.
 *
 * \warning Задания, у которых не требуется обработка вложенности, выполняются
 * в первую очередь! Они не передаются в метод proceed_nested_task_list()!
 *
 * \param [in] task_list Список заданий, который требуется обработать
 *
 * \return Результат обработки списка заданий
 */
json TaskManager::proceed_task_list(const json &task_list) {
    json result;
    json nested_result;

    std::vector<json> nested_task_list{};
    std::vector<json> task_cache = task_list.get<std::vector<json>>();

    for (const json &task : task_cache) {
        if (stop_requested) {
            logger::log(LEVEL_WARN, "Task list proceeding stopped");
            device_set.reset_stop_request();
            stop_requested = false;

            result[WORD_RESULT] = {
                    {WORD_RESULT_ID, MEASUREMENTS_STOPS_ID},
                    {WORD_RESULT_MSG, MEASUREMENTS_STOPS_MSG},
                    {WORD_RESULT_DATA, json::value_t::null}
            };

            return result;
        }

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
    nested_result = proceed_nested_task_list(std::move(nested_task_list));

    if (nested_result[WORD_RESULT][WORD_RESULT_ID] != 0) {
        return nested_result;
    } else {
        result[WORD_RESULT][WORD_RESULT_DATA] = nested_result[WORD_RESULT][WORD_RESULT_DATA];
    }

    return result;
}

/**
 * \brief Метод, позволяющий произвести обработку списка заданий, с учётом вложенности
 *
 * \param [in] nested_task_list Список заданий, имеющих вложенность
 *
 * \return Результат обработки данных
 */
json TaskManager::proceed_nested_task_list(std::vector<json> nested_task_list) {
    json result;
    std::string data{};
    std::string acquired_data{};

    for (json &nested_task : nested_task_list) {
        if (stop_requested) {
            logger::log(LEVEL_WARN, "Nested task list proceeding stopped");
            device_set.reset_stop_request();
            stop_requested = false;

            result[WORD_RESULT] = {
                    {WORD_RESULT_ID, MEASUREMENTS_STOPS_ID},
                    {WORD_RESULT_MSG, MEASUREMENTS_STOPS_MSG},
                    {WORD_RESULT_DATA, json::value_t::null}
            };

            return result;
        }

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
        if (stop_requested) {
            logger::log(LEVEL_WARN, "Nested task list proceeding stopped");
            device_set.reset_stop_request();
            stop_requested = false;

            result[WORD_RESULT] = {
                    {WORD_RESULT_ID, MEASUREMENTS_STOPS_ID},
                    {WORD_RESULT_MSG, MEASUREMENTS_STOPS_MSG},
                    {WORD_RESULT_DATA, json::value_t::null}
            };

            return result;
        }

        if (nested_task_list[nested_pos][WORD_TASK_TYPE] == TASK_TYPE_GET_DATA) {
            acquired_data = std::move(get_data_task(nested_task_list[nested_pos][WORD_TASK_ARGS]));

            if (stop_requested) {
                logger::log(LEVEL_WARN, "Nested task list proceeding stopped");
                device_set.reset_stop_request();
                stop_requested = false;

                result[WORD_RESULT] = {
                        {WORD_RESULT_ID, MEASUREMENTS_STOPS_ID},
                        {WORD_RESULT_MSG, MEASUREMENTS_STOPS_MSG},
                        {WORD_RESULT_DATA, json::value_t::null}
                };

                return result;
            }

            if (!acquired_data.empty()) {
                data += (data.empty() ? "" : ";") + acquired_data;
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
                    case FREQ_MOVE_OK:
                        nested_pos = -1;
                        continue;
                    case FREQ_MOVE_BOUND:
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
                case ANGLE_MOVE_OK:
                    nested_pos = -1;
                    continue;
                case ANGLE_MOVE_BOUND:
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

/**
 * \brief Метод, производящий обработку принятого JSON-объекта
 *
 * Проверяется, какой тип данных был получен - задание или список заданий, а затем,
 * передаёт JSON-объект в соответствующий метод для дальнейшей обработки. Также, в
 * методе производится замер времени выполнения задания/списка заданий.
 *
 * \param [in] data Принятый JSON-объект
 *
 * \return Результат обработки принятого объекта
 */
std::string TaskManager::proceed(const json &data) {
    json answer;

    auto start_time = std::chrono::high_resolution_clock::now();

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

    auto stop_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();

    auto duration_ms = duration % 1000;
    duration = (long long) ((duration - duration_ms) / 1000);

    auto duration_s = duration % 60;
    duration = (long long) ((duration - duration_s) / 60);

    logger::log(LEVEL_INFO, "Proceeding finished for {}:{:02}.{:03}", duration, duration_s, duration_ms);

    return to_string(answer);
}

/**
 * \brief Метод, проверяющий, принадлежит тип принятого задания типу TASK_TYPE_STOP
 *
 * \param [in] data Принятое задание
 *
 * \return Если тип принятого задания равен TASK_TYPE_STOP, возвращается true.
 * В противном случае - false.
 */
bool TaskManager::received_stop_task(const json &data) {
    if (data.contains(WORD_TASK) && data[WORD_TASK][WORD_TASK_TYPE] == TASK_TYPE_STOP) {
        return true;
    }

    return false;
}

/**
 * \brief Метод, проверяющий, принадлежит тип принятого задания типу TASK_TYPE_DISCONNECT
 *
 * \param [in] data Принятое задание
 *
 * \return Если тип принятого задания равен TASK_TYPE_DISCONNECT, возвращается true.
 * В противном случае - false.
 */
bool TaskManager::received_disconnect_task(const json &data) {
    if (data.contains(WORD_TASK) && data[WORD_TASK][WORD_TASK_TYPE] == TASK_TYPE_DISCONNECT) {
        return true;
    }

    return false;
}

/**
 * \brief Присваивает флагу stop_request значение true, тем самым, останавливая
 * процес измерения
 */
void TaskManager::request_stop() {
    device_set.request_stop();
    stop_requested = true;
}
