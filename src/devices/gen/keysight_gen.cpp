/**
 * \file
 * \brief Файл исходного кода, в котором реализованы методы для класа KeysightGen
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#include "keysight_gen.hpp"

/**
 * \brief Вызов родительского конструктора
 *
 * \param [in] device_address Адрес устройства, который передаётся в родительский
 * конструктор
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * KeysightGen keysight_gen("TCPIP0::localhost::5025::SOCKET");
 * \endcode
 */
KeysightGen::KeysightGen(std::string device_address) : GenDevice(std::move(device_address)) {}

/**
 * \brief Сброс настроек
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->preset();
 * \endcode
 */
void KeysightGen::preset() {
    send("SYSTEM:PRESET");

    send(":MODULATION:STATE OFF");
    send("OUTPUT:STATE OFF");
}

/**
 * \brief Установка определённого значения частоты
 *
 * \param [in] freq Требуемая частота
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_freq(1.2e9);
 * \endcode
 */
void KeysightGen::set_freq(double freq) {
    start_freq = freq;
    stop_freq = freq;

    current_freq = freq;

    points = 1;
    current_point = 0;

    send_wait(":FREQ {}", current_freq);
}

/**
 * \brief Установка частотного диапазона
 *
 * \param [in] start_freq Начальное значение частоты
 * \param [in] stop_freq Конечное значение частоты
 * \param [in] points Количество точек
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_freq_range(1.2e9, 2.4e9, 201);
 * \endcode
 */
void KeysightGen::set_freq_range(double start_freq, double stop_freq, int points) {
    this->start_freq = start_freq;
    this->stop_freq = stop_freq;

    this->points = points;

    freq_step = this->points <= 1 ? 0 : (this->stop_freq - this->start_freq) / (this->points - 1);

    current_freq = start_freq;
    current_point = 0;
}

/**
 * \brief Установка требуемой мощности сигнала
 *
 * \param [in] power Требуемое значение мощности
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_power(-10.0);
 * \endcode
 */
void KeysightGen::set_power(float power) {
    this->power = power;

    send(":SOURCE:POWER {}", this->power);
}

/**
 * \brief Отключает порт
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->rf_off();
 * \endcode
 */
void KeysightGen::rf_off() {
    send("OUTPUT:STATE OFF");
}

/**
 * \brief Включает порт
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->rf_on();
 * \endcode
 */
void KeysightGen::rf_on() {
    send("OUTPUT:STATE ON");
}

/**
 * \brief Переход к следующей частотной точке
 *
 * \return Если следующая частотная точка находится в пределах
 * границ изменения, то возвращает FREQ_MOVE_OK. Если точка
 * находится на границе, то возвращает FREQ_MOVE_BOUND.
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_freq_range(1.2e9, 2.4e9, 201);
 *
 * while (gen.next_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана следующая частотная точка" << std::endl;
 * }
 * \endcode
 */
int KeysightGen::next_freq() {
    if (current_point == points - 1) {
        return FREQ_MOVE_BOUND;
    }

    rf_off();

    current_freq += freq_step;
    ++current_point;

    send_wait(":FREQ {}", current_freq);
    send(":FREQ?");

    return FREQ_MOVE_OK;
}

/**
 * \brief Переход к предыдущей частотной точке
 *
 * \return Если предыдущая частотная точка находится в пределах
 * границ изменения, то возвращает FREQ_MOVE_OK. Если точка
 * находится на границе, то возвращает FREQ_MOVE_BOUND.
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_freq_range(1.2e9, 2.4e9, 201);
 *
 * while (gen.prev_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана предыдущая частотная точка" << std::endl;
 * }
 * \endcode
 */
int KeysightGen::prev_freq() {
    if (current_point == 0) {
        return FREQ_MOVE_BOUND;
    }

    rf_off();

    current_freq -= freq_step;
    --current_point;

    send_wait(":FREQ {}", current_freq);

    return FREQ_MOVE_OK;
}

/**
 * \brief Устанавливает частотную точку, соответствующую началу
 * диапазона.
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_freq_range(1.2e9, 2.4e9, 201);
 *
 * while (gen.next_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана следующая частотная точка" << std::endl;
 * }
 *
 * gen.move_to_start_freq();
 * \endcode
 */
void KeysightGen::move_to_start_freq() {
    rf_off();

    current_freq = start_freq;
    current_point = 0;

    send_wait(":FREQ {}", current_freq);
}

/**
 * \brief Устанавливает частотную точку, соответствующую началу
 * диапазона.
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_freq_range(1.2e9, 2.4e9, 201);
 *
 * while (gen.prev_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана предыдущая частотная точка" << std::endl;
 * }
 *
 * gen.move_to_stop_freq();
 * \endcode
 */
void KeysightGen::move_to_stop_freq() {
    rf_off();

    current_freq = stop_freq;
    current_point = points - 1;

    send_wait(":FREQ {}", current_freq);
}

/**
 * \brief Получает текущее значение частоты сигнала
 *
 * \return Значение частоты сигнала
 *
 * **Пример**
 * \code
 * GenDevice *gen = new KeysightGen("TCPIP0::localhost::5025::SOCKET");
 * gen->set_freq_range(1.2e9, 2.4e9, 201);
 *
 * while (gen.prev_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "f = " << gen.get_current_freq() << std::endl;
 * }
 * \endcode
 */
double KeysightGen::get_current_freq() {
    std::string current_freq = send(":FREQ?");
    return std::stod(current_freq);
}

