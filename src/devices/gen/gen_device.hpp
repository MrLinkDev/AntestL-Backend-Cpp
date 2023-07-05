/**
 * \file
 * \brief Заголовочный файл, в котором представлен класс GenDevice и определён
 * набор констант
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_GEN_DEVICE_HPP
#define ANTESTL_BACKEND_GEN_DEVICE_HPP

#include "../visa_device.hpp"
#include "../../utils/exceptions.hpp"

/// Стандартное значение мощности сигнала
#define DEFAULT_GEN_POWER           -20

/// Стандартная начальная частота
#define DEFAULT_GEN_START_FREQ      1000000000
/// Стандартная конечная частота
#define DEFAULT_GEN_STOP_FREQ       9000000000
/// Стандартное число точек
#define DEFAULT_GEN_POINTS          201

/// Стандартная текущая частота
#define DEFAULT_CURRENT_FREQ        0
/// Стандартный частотный шаг
#define DEFAULT_FREQ_STEP           0

/// Стандартная текущая точка
#define DEFAULT_CURRENT_POINT       0

/// Статус изменения частотной точки: находится в пределах допустимых границ
#define FREQ_MOVE_OK                0xF0
/// Статус изменения частотной точки: находится на границе допустимых значений
#define FREQ_MOVE_BOUND             0xF1

/**
 * \brief Класс, в котором представлены методы для управления генератором
 */
class GenDevice : public VisaDevice {

protected:
    /// Мощность сигнала
    float power         = DEFAULT_GEN_POWER;

    /// Начальная частота
    double start_freq   = DEFAULT_GEN_START_FREQ;
    /// Конечная частота
    double stop_freq    = DEFAULT_GEN_STOP_FREQ;

    /// Ширина шага по частоте
    double freq_step    = DEFAULT_FREQ_STEP;
    /// Текущая частота
    double current_freq = DEFAULT_CURRENT_FREQ;

    /// Количество точек
    int points          = DEFAULT_GEN_POINTS;
    /// Текущая точка
    int current_point   = DEFAULT_CURRENT_POINT;

public:
    GenDevice() = default;

    /**
     * \brief Конструктор, в котором реализовано создание объекта с определённым
     * адресом, а после - осуществляет подключение к генератору
     *
     * \param [in] device_address Адрес устройства
     */
    explicit GenDevice(std::string device_address) : VisaDevice(std::move(device_address)) {
        this->connect();

        if (idn().empty()) {
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
     * \brief Установка определённого значения частоты
     *
     * \param [in] freq Требуемая частота
     */
    virtual void set_freq(double freq) {};

    /**
     * \brief Установка частотного диапазона
     *
     * \param [in] start_freq Начальное значение частоты
     * \param [in] stop_freq Конечное значение частоты
     * \param [in] points Количество точек
     */
    virtual void set_freq_range(double start_freq, double stop_freq, int points) {};

    /**
     * \brief Установка требуемой мощности сигнала
     *
     * \param [in] power Требуемое значение мощности
     */
    virtual void set_power(float power) {};

    /**
     * \brief Отключает порт
     */
    virtual void rf_off() {};

    /**
     * \brief Включает порт
     */
    virtual void rf_on() {};

    /**
     * \brief Переход к следующей частотной точке
     *
     * \return Если следующая частотная точка находится в пределах
     * границ изменения, то возвращает FREQ_MOVE_OK. Если точка
     * находится на границе, то возвращает FREQ_MOVE_BOUND.
     */
    virtual int next_freq() {return FREQ_MOVE_BOUND;};

    /**
     * \brief Переход к предыдущей частотной точке
     *
     * \return Если предыдущая частотная точка находится в пределах
     * границ изменения, то возвращает FREQ_MOVE_OK. Если точка
     * находится на границе, то возвращает FREQ_MOVE_BOUND.
     */
    virtual int prev_freq() {return FREQ_MOVE_BOUND;};

    /**
     * \brief Устанавливает частотную точку, соответствующую началу
     * диапазона.
     */
    virtual void move_to_start_freq() {};

    /**
     * \brief Устанавливает частотную точку, соответствующую началу
     * диапазона.
     */
    virtual void move_to_stop_freq() {};

    /**
     * \brief Получает текущее значение частоты сигнала
     *
     * \return Значение частоты сигнала
     */
    virtual double get_current_freq() {return 0.0;};
};

#endif //ANTESTL_BACKEND_GEN_DEVICE_HPP
