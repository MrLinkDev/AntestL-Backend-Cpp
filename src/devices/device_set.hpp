/**
 * \file
 * \brief Заголовочный файл, содержащий в себе класс DeviceSet и набор
 * констант, необходимых для данного класса.
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_DEVICE_SET_HPP
#define ANTESTL_BACKEND_DEVICE_SET_HPP

#include <algorithm>
#include "vna/vna_device.hpp"
#include "gen/gen_device.hpp"
#include "rbd/rbd_device.hpp"
#include "vna/keysight_m9807a.hpp"
#include "gen/keysight_gen.hpp"
#include "rbd/tesart_rbd.hpp"

/// Тип устройства: ВАЦ
#define DEVICE_VNA  0xD0
/// Тип устройства: генератор
#define DEVICE_GEN  0xD1
/// Тип устройства: ОПУ
#define DEVICE_RBD  0xD2

/// Разделитель столбцов
#define COLUMN_DELIMITER    ","
/// Разделитель строк
#define ROW_DELIMITER       ";"

/**
 * \brief Структура, которая является вектором iq с набором методов для работы с ней.
 *
 * Структура определена для хранения данных измерения, полученных от ВАЦ для порта.
 */
struct iq_port_list_t {
    /// Данные для одного порта
    iq_port_data_t iq_port_list;

    /// Конструктор, в котором на вход поступает объект iq, который добавляется в вектор iq
    iq_port_list_t(iq iq_item) {
        insert_item(std::move(iq_item));
    }

    /**
     * \brief Метод, добавляющий объект iq в вектор
     *
     * \param [in] iq_item Комплексная точка
     */
    void insert_item(iq iq_item) {
        iq_port_list.push_back(std::move(iq_item));
    }

    /**
     * \brief Метод, который преобразует вектор iq в строку
     *
     * \return Строка, в которой содержатся данные измерения, для порта ВАЦ
     */
    std::string to_string() {
        std::string result{};

        for (const iq &item : iq_port_list) {
            result += (result.empty() ? "" : COLUMN_DELIMITER) + item.i + COLUMN_DELIMITER + item.q;
        }

        return result;
    }
};

/**
 * \brief Структура, которая содержит в себе данные, полученные при одиночном вызове
 * метода get_data().
 *
 * Структура содержит в себе набор объектов для хранения данных, методы для добавления
 * данных и их преобразования.
 */
struct data_t {
    /// Вектор, который содержит в себе значение угла для полученных точек
    std::vector<std::string> angle_list{};
    /// Вектор, который содержит в себе значение частоты для полученных данных
    std::vector<double> freq_list{};

    /// Вектор, содержащий в себе полученные данные при измерении всех портов ВАЦ
    std::vector<iq_port_list_t> iq_data_list{};

    /**
     * \brief Добавляет значения углов в соответствующий вектор
     *
     * \param [in] angles Список углов, полученный при проведении измерения
     */
    void insert_angles(std::string angles) {
        angle_list.push_back(std::move(angles));
    }

    /**
     * \brief Добавляет значение частоты в соответствующий вектор
     *
     * \param [in] freq Значение частоты, полученное при проведении измерения
     */
    void insert_freq(double freq) {
        freq_list.push_back(freq);
    }

    /**
     * \brief Добавляет значения частот в соответствующий вектор
     *
     * \param [in] freq_list Значения частот, полученные при проведении измерения
     */
    void insert_freq_list(std::vector<double> freq_list) {
        this->freq_list = std::move(freq_list);
    }

    /**
     * \brief Добавляет полученные после проведения измерения данные в
     * соответстующий вектор
     *
     * \param [in] iq_port_data Полученные данные для одного порта ВАЦ
     */
    void insert_iq_port_data(iq_port_data_t iq_port_data) {
        if (iq_data_list.empty()) {
            for (const iq &item : iq_port_data) {
                iq_data_list.push_back(iq_port_list_t(item));
            }
        } else {
            for (int pos = 0; pos < iq_port_data.size(); ++pos) {
                iq_data_list[pos].insert_item(iq_port_data[pos]);
            }
        }
    }

    /**
     * \brief Преобразует структуру в строку
     *
     * \return Строка, в которой содержатся данные, полученные при проведении измерения
     * для всех портов ВАЦ.
     */
    std::string to_string() {
        std::string result{};

        for (int pos = 0; pos < iq_data_list.size(); ++pos) {
            if (angle_list.size() == 1 && angle_list[0].empty()) {
                result +=
                        (result.empty() ? "" : ROW_DELIMITER) +
                        std::to_string(freq_list.size() > 1 ? freq_list[pos] : freq_list[0]) + COLUMN_DELIMITER +
                        iq_data_list[pos].to_string();
            } else {
                result +=
                        (result.empty() ? "" : ROW_DELIMITER) +
                        (angle_list.size() > 1 ? angle_list[pos] : angle_list[0]) + COLUMN_DELIMITER +
                        std::to_string(freq_list.size() > 1 ? freq_list[pos] : freq_list[0]) + COLUMN_DELIMITER +
                        iq_data_list[pos].to_string();
            }
        }

        return result;
    }
};

/**
 * \brief Класс набора устройств, в котором реализованы методы взаимодействия
 * между устройствами
 */
class DeviceSet {
    /// Указатель на объект ВАЦ
    VnaDevice *vna = nullptr;
    /// Указатель на объект внешнего генератора
    GenDevice *ext_gen = nullptr;
    /// Указатель на объект ОПУ
    RbdDevice *rbd = nullptr;

    /// Тип измерения. По-умолчанию выбрано измерение коэффициента передачи
    int meas_type = MEAS_TRANSITION;
    /// Флаг, показывающий, используется ли внешний генератор
    bool using_ext_gen = false;

    /// Флаг, показывающий, сконфигурированы ли трассы заранее
    bool traces_configured = false;
    /// Флаг, показывающий, был ли получен запрос на остановку измерений
    bool stop_requested = false;

public:
    DeviceSet() = default;

    bool connect(int device_type, std::string device_model, const std::string &device_address);
    void disconnect();

    bool configure(int meas_type, float rbw, int source_port, bool using_ext_gen);

    bool set_power(float power);

    bool set_freq(double freq);
    bool set_freq_range(double start_freq, double stop_freq, int points);

    int next_freq();
    int prev_freq();

    bool move_to_start_freq();

    double get_current_freq();
    std::vector<double> get_freq_list();

    bool set_angle(float angle, int axis_num);
    bool set_angle_range(float start_angle, float stop_angle, int points, int axis_num);

    int next_angle(int axis_num);
    int prev_angle(int axis_num);

    bool move_to_start_angle(int axis_num);

    std::string get_current_angles();

    bool set_path(std::vector<int> path_list);
    int get_vna_switch_module_count();

    bool is_using_ext_gen() const;

    data_t get_data(std::vector<int> port_list);

    void request_stop();

    void reset_stop_request();
};


#endif //ANTESTL_BACKEND_DEVICE_SET_HPP
