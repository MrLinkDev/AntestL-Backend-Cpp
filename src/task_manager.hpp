/**
 * \file
 * \brief Заголовочный файл, в котором определён класс TaskManager и набор констант
 * для работы с ним
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */



#ifndef ANTESTL_BACKEND_TASK_MANAGER_HPP
#define ANTESTL_BACKEND_TASK_MANAGER_HPP

#include "json.hpp"
#include "devices/device_set.hpp"

/// Ключ, значением которого является объект задания
#define WORD_TASK                   "task"
/// Ключ, значением которого является объект список заданий
#define WORD_TASK_LIST              "task_list"

/// Ключ, значением которого является тип задания
#define WORD_TASK_TYPE              "type"
/// Ключ, значением которого является объект аргументов для задания
#define WORD_TASK_ARGS              "args"

/// Ключ, значением которого является уровень вложенности
#define WORD_NESTED                 "nested"

/// Ключ, значением которого является номер оси ОПУ
#define WORD_AXIS                   "axis"

/// Тип задания: подключение
#define TASK_TYPE_CONNECT           "connect"
/// Тип задания: настройка
#define TASK_TYPE_CONFIGURE         "configure"

/// Тип задания: установка мощности
#define TASK_TYPE_SET_POWER         "set_power"

/// Тип задания: установка частоты
#define TASK_TYPE_SET_FREQ          "set_freq"
/// Тип задания: установка частотного диапазона
#define TASK_TYPE_SET_FREQ_RANGE    "set_freq_range"
/// Тип задания: переход на следующую частотную точку
#define TASK_TYPE_NEXT_FREQ         "next_freq"

/// Тип задания: установка угла
#define TASK_TYPE_SET_ANGLE         "set_angle"
/// Тип задания: установка углового диапазона
#define TASK_TYPE_SET_ANGLE_RANGE   "set_angle_range"
/// Тип задания: переход на следующую угловую точку
#define TASK_TYPE_NEXT_ANGLE        "next_angle"

/// Тип задания: изменение положений переключателей
#define TASK_TYPE_CHANGE_PATH       "set_path"

/// Тип задания: проведение измерения и сбор данных
#define TASK_TYPE_GET_DATA          "get_data"

/// Тип задания: остановка
#define TASK_TYPE_STOP              "stop"

/// Тип задания: отключение
#define TASK_TYPE_DISCONNECT        "disconnect"

/// Тип устройства: внешний генератор
#define DEVICE_EXT_GEN              "ext_gen"
/// Тип устройства: ОПУ УПКБ
#define DEVICE_RBD_UPKB             "upkb_rbd"
/// Тип устройства: ОПУ ТЕСАРТа
#define DEVICE_RBD_TESART           "tesart_rbd"
/// Тип устройства: Демо ОПУ
#define DEVICE_RBD_DEMO             "demo_rbd"

/// Ключ, значение которого - объект результата
#define WORD_RESULT                 "result"
/// Ключ, значение которого - идентификатор состояния результата работы
#define WORD_RESULT_ID              "id"
/// Ключ, значение которого - сообщение о состоянии результата работы
#define WORD_RESULT_MSG             "message"
/// Ключ, значение которого - данные, полученные в результате работы
#define WORD_RESULT_DATA            "data"

/// Идентификатор: задание выполнено успешно
#define RESULT_OK_ID                0x00
/// Сообщение: задание выполнено успешно
#define RESULT_OK_MSG               "Complete"

/// Идентификатор: невозможно подключиться к ВАЦ
#define VNA_NO_CONNECTION_ID        0x01
/// Сообщение: невозможно подключиться к ВАЦ
#define VNA_NO_CONNECTION_MSG       "No connection with vna"

/// Идентификатор: невозможно подключиться к внешнему генератору
#define EXT_GEN_NO_CONNECTION_ID    0x02
/// Сообщение: невозможно подключиться к внешнему генератору
#define EXT_GEN_NO_CONNECTION_MSG   "No connection with external generator"

/// Идентификатор: невозможно подключиться к ОПУ
#define RBD_NO_CONNECTION_ID        0x03
/// Сообщение: невозможно подключиться к ОПУ
#define RBD_NO_CONNECTION_MSG       "No connection with RBD"

/// Идентификатор: невозможно настроить ВАЦ
#define VNA_CONFIGURE_ERR_ID        0x10
/// Сообщение: невозможно настроить ВАЦ
#define VNA_CONFIGURE_ERR_MSG       "Can't configure VNA"

/// Идентификатор: невозможно изменить мощность
#define ERR_SET_POWER_ID            0x20
/// Сообщение: невозможно изменить мощность
#define ERR_SET_POWER_MSG           "Can't set power"

/// Идентификатор: невозможно установить частоту
#define ERR_SET_FREQ_ID             0x30
/// Сообщение: невозможно установить частоту
#define ERR_SET_FREQ_MSG            "Can't set frequency"

/// Идентификатор: невозможно установить частотный диапазон
#define ERR_SET_FREQ_RANGE_ID       0x31
/// Сообщение: невозможно установить частотный диапазон
#define ERR_SET_FREQ_RANGE_MSG      "Can't set frequency range"

/// Идентификатор: невозможно установить угол
#define ERR_SET_ANGLE_ID            0x40
/// Сообщение: невозможно установить угол
#define ERR_SET_ANGLE_MSG           "Can't set angle"

/// Идентификатор: невозможно установить диапазон изменения угла
#define ERR_SET_ANGLE_RANGE_ID      0x41
/// Сообщение: невозможно установить диапазон изменения угла
#define ERR_SET_ANGLE_RANGE_MSG     "Can't set angle range"

/// Идентификатор: невозможно изменить положение переключателей
#define ERR_CHANGE_SWITCH_PATH_ID   0x50
/// Сообщение: невозможно изменить положение переключателей
#define ERR_CHANGE_SWITCH_PATH_MSG  "Can't change switch path"

/// Идентификатор: невозможно провести измерение или (и) собрать данные с ВАЦ
#define ERR_GETTING_DATA_ID         0x60
/// Сообщение: невозможно провести измерение или (и) собрать данные с ВАЦ
#define ERR_GETTING_DATA_MSG        "Can't acquire data from VNA"

/// Идентификатор: Измерение остановлено
#define MEASUREMENTS_STOPS_ID       0xA0
/// Сообщение: Измерение остановлено
#define MEASUREMENTS_STOPS_MSG      "Measurements stopped"

/// Идентификатор: неизвестный тип задания
#define WRONG_TASK_TYPE_ID          0xFE
/// Сообщение: неизвестный тип задания
#define WRONG_TASK_TYPE_MSG         "Wrong task type"

/// Идентификатор: не было обнаружено заданий или списков заданий
#define NO_TASK_ID                  0xFF
/// Сообщение: не было обнаружено заданий или списков заданий
#define NO_TASK_MSG                 "No task or task list"

using namespace nlohmann;

/**
 * \brief Класс, в котором реализованы методы для выполнения заданий или списков
 * заданий
 */
class TaskManager {
    /// Набор устройств
    DeviceSet device_set;

    /// Флаг, показывающий требуется ли остановка измерений или нет
    bool stop_requested = false;

    json connect_task(json device_list);

    void disconnect_task();

    bool configure_task(json config_params);

    bool set_power_task(json power_value);

    bool set_freq_task(json freq_value);
    bool set_freq_range_task(json freq_range);

    int next_freq_task();

    bool set_angle_task(json angle_value);
    bool set_angle_range_task(json angle_range);

    int next_angle_task(int axis_num);

    bool set_path_task(json path_values);

    std::string get_data_task(json port_list);

    json proceed_task(const json &task);
    json proceed_task_list(const json& task_list);

    json proceed_nested_task_list(std::vector<json> nested_task_list);

public:
    TaskManager() = default;

    std::string proceed(const json &data);

    bool received_stop_task(const json &data);

    bool received_disconnect_task(const json &data);

    void request_stop();
};


#endif //ANTESTL_BACKEND_TASK_MANAGER_HPP
