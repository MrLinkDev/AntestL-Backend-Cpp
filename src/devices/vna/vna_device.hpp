/**
 * \file
 * \brief Заголовочный файл, в котором реализован класс VnaDevice
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_VNA_DEVICE_HPP
#define ANTESTL_BACKEND_VNA_DEVICE_HPP

#include "../visa_device.hpp"
#include "../../utils/exceptions.hpp"

#include <vector>

/// Стандартная начальная частота для ВАЦ
#define DEFAULT_VNA_START_FREQ      1000000000
/// Стандартная конечная частота для ВАЦ
#define DEFAULT_VNA_STOP_FREQ       9000000000

/// Стандартная полоса разрешающего фильтра
#define DEFAULT_VNA_RBW             1000

/// Стандартное количество точек трассы
#define DEFAULT_VNA_POINTS          201

/// Стандартная мощность зондирующего порта
#define DEFAULT_VNA_POWER           -20;

/// Стандартный номер зондирующего порта
#define DEFAULT_VNA_SOURCE_PORT     1

/// Тип измерения: коэффициент передачи
#define MEAS_TRANSITION             0x00
/// Тип измерения: коэффициент отражения
#define MEAS_REFLECTION             0x01

/// Разделитель данных, принимаемых от ВАЦ
#define DATA_DELIMITER              ','

/// Структура, содержащая в себе значения квадратур в определённой точке
struct iq {
    std::string i{};
    std::string q{};
};

typedef std::vector<iq> iq_port_data_t;

/**
 * \brief Класс, в котором реализованы методы для работы с ВАЦ
 */
class VnaDevice : public VisaDevice {

protected:
    /// Начальная частота
    double start_freq = DEFAULT_VNA_START_FREQ;
    /// Конечная частота
    double stop_freq = DEFAULT_VNA_STOP_FREQ;

    /// Полоса разрешающего фильтра
    float rbw = DEFAULT_VNA_RBW;
    /// Количество точек трассы
    int points = DEFAULT_VNA_POINTS;

    /// Размер частотного шага
    double freq_step = (stop_freq - start_freq) / (points - 1);

    /// Мощность зондирующего порта
    float power = DEFAULT_VNA_POWER;

    /// Номер зондирующего порта
    int source_port = DEFAULT_VNA_SOURCE_PORT;
    /// Тип измерения
    int meas_type = MEAS_TRANSITION;

public:
    VnaDevice() = default;

    /**
     * \brief Конструктор, в котором реализовано создание объекта с определённым
     * адресом, а после - осуществляет подключение к ВАЦ
     *
     * \param [in] device_address Адрес устройства
     */
    explicit VnaDevice(std::string device_address) : VisaDevice(std::move(device_address)){
        this->connect();

        if (!this->is_connected() || idn().empty()) {
            this->connected = false;

            logger::log(LEVEL_ERROR, NO_CONNECTION_MSG);
            throw antestl_exception(NO_CONNECTION_MSG, NO_CONNECTION_CODE);
        }
    }

    /**
     * \brief Сброс настроек
     */
    virtual void preset() {};

    /**
     * \brief Полный сброс настроек
     */
    virtual void full_preset() {};

    /**
     * \brief Инициализация канала для измерений
     */
    virtual void init_channel() {};

    /**
     * \brief Настройка ВАЦ для измерения
     *
     * \param [in] meas_type Тип измерения
     * \param [in] rbw Ширина разрешающего фильтра
     * \param [in] source_port Номер зондирующего порта
     */
    virtual void configure(int meas_type, float rbw, int source_port) {};

    /**
     * \brief Создание необходимых трасс, в зависимости от того, используется ли внешний
     * генератор
     *
     * \param [in] port_list Список портов, для которых неободимо создать трассы
     * \param [in] external Флаг, который показывает используется ли внешний генератор
     */
    virtual void create_traces(std::vector<int> port_list, bool external) {};

    /**
     * \brief Установка требуемой мощности зондирующего порта
     *
     * \param [in] power Требуемое значение мощности
     */
    virtual void set_power(float power) {};

    /**
     * \brief Установка частотного диапазона
     *
     * \param [in] start_freq Начальное значение частоты
     * \param [in] stop_freq Конечное значение частоты
     * \param [in] points Количество точек
     */
    virtual void set_freq_range(double start_freq, double stop_freq, int points) {};

    /**
     * \brief Установка определённого значения частоты
     *
     * \param freq Требуемая частота
     */
    virtual void set_freq(double freq) {};

    /**
     * \brief Установка положений переключателей в определённые позиции
     *
     * \param [in] path_list 
     */
    virtual void set_path(std::vector<int> path_list) {};

    /**
     * \brief Запрос количества переключателей, подключенных к ВАЦ
     *
     * \return Количество переключателей
     */
    virtual int get_switch_module_count() {return 0;}

    /**
     * \brief Отключает все порты
     */
    virtual void rf_off() {};

    /**
     * \brief Отключает выбранный порт
     *
     * \param [in] port Порт, который будет отключен
     */
    virtual void rf_off(int port) {};

    /**
     * \brief Включает все порты
     */
    virtual void rf_on() {};

    /**
     * \brief Включает выбранный порт
     *
     * \param [in] port Порт, который будет включен
     */
    virtual void rf_on(int port) {};

    /**
     * \brief Перевод триггера в режим manual
     */
    virtual void trigger() {};

    /**
     * \brief Запуск измерения
     */
    virtual void init() {};

    /**
     * \brief Сбор данных, полученных в результате измерения, для одного порта
     *
     * \param [in] trace_index Индекс трассы, которая соответствует требуемому порту
     *
     * \return Результат измерений одного порта
     */
    virtual iq_port_data_t get_data(int trace_index) {return iq_port_data_t{};};

    /**
     * \brief Запрос номера зондирующего порта
     *
     * \return Номер зондирующего порта
     */
    int get_source_port() const {
        return source_port;
    };

    /**
     * \brief Запрос начальной частоты диапазона
     *
     * \return Значение начальной частоты
     */
    double get_start_freq() const {
        return start_freq;
    }

    /**
     * \brief Запрос ширины шага по частоте
     *
     * \return Ширина шага по частоте
     */
    double get_freq_step() const {
        return freq_step;
    }

    /**
     * \brief Запрос количества точек трассы
     *
     * \return Количество точек
     */
    int get_points() const {
        return points;
    }
};


#endif //ANTESTL_BACKEND_VNA_DEVICE_HPP
