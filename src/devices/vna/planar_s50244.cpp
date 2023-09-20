/**
 * \file
 * \brief Файл исходного кода, в котором реализованы методы для класа PlanarS50244
 *
 * \author Александр Горбунов
 * \date 19 сентября 2023
 */

#include <thread>
#include "planar_s50244.h"
#include "../../utils/string_utils.hpp"
#include "../../utils/array_utils.hpp"

/**
 * \brief Вызов родительского конструктора
 *
 * \param [in] device_address Адрес устройства, который передаётся в родительский
 * конструктор
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 * PlanarS50244 keysight_vna("TCPIP0::localhost::5025::SOCKET");
 * \endcode
 */
PlanarS50244::PlanarS50244(std::string device_address) : VnaDevice(std::move(device_address)) {}

/**
 * \brief Сброс настроек
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 * vna->preset();
 * \endcode
 */
void PlanarS50244::preset() {
    send(":SYSTEM:PRESET");
}

/**
  * \brief Полный сброс настроек
  *
  * **Пример**
  * \code
  * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
  * vna->full_preset();
  * \endcode
  */
void PlanarS50244::full_preset() {
    send("*CLS");
    send("*RST");

    send(":SYSTEM:PRESET");
    send(":FORMAT:DATA ASCII");
}

/**
 * \brief Инициализация канала для измерений
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 * \endcode
 */
void PlanarS50244::init_channel() {

    send_wait_err(":SOURce:POWer:PORT:COUPle 0");
    send_wait_err(":OUTPUT:STATE OFF");

    send_wait_err("TRIG:SEQ:SOUR MAN");
    //send_wait_err("SENS:SWE:MODE CONT");

    send_wait_err(":DISPlay:SPLIT 1");
    send_wait_err(":DISPlay:WINDOW1:ACTIVATE");

    send_wait_err("INITiate:CONTinuous OFF");
}

/**
 * \brief Настройка ВАЦ для измерения
 *
 * \param [in] meas_type Тип измерения
 * \param [in] rbw Ширина разрешающего фильтра
 * \param [in] source_port Номер зондирующего порта
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 * \endcode
 */
void PlanarS50244::configure(int meas_type, float rbw, int source_port) {
    this->meas_type = meas_type;

    this->rbw = rbw;
    this->source_port = source_port;

    send_wait_err(":TRIGger:SEQuence:SCOPe ACTive");
    send_wait_err("TRIG:SCOP ALL");

    send_wait_err(":SENSe:BANDwidth:RESolution {}", this->rbw);
}

/**
 * \brief Создание необходимых трасс, в зависимости от того, используется ли внешний
 * генератор
 *
 * \param [in] port_list Список портов, для которых неободимо создать трассы
 * \param [in] external Флаг, который показывает используется ли внешний генератор
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 *
 * vna->create_traces({2, 4, 5}, false);
 * \endcode
 */
void PlanarS50244::create_traces(std::vector<int> port_list, bool external) {
    std::string trace_name{};
    std::string trace_params{};

    for (int pos = 0; pos < port_list.size(); ++pos) {
        if (meas_type == MEAS_TRANSITION) {
            trace_name = std::format("S{}{}", port_list[pos], source_port);
        } else {
            trace_name = std::format("S{}{}", port_list[pos], port_list[pos]);
        }

        send_wait_err(R"(:CALCulate1:PARameter:DEFine {})", trace_name);
        send_wait_err(":DISPlay:WINDow:TRACe1:STATe ON");
    }
}

/**
 * \brief Установка требуемой мощности зондирующего порта
 *
 * \param [in] power Требуемое значение мощности
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->set_power(-10.0);
 * \endcode
 */
void PlanarS50244::set_power(float power) {
    this->power = power;

    for (int port = 1; port < S50244_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer:PORT{}:LEVel:IMMediate:AMPLitude {}", port, this->power);
    }
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
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 * vna->set_freq_range(1.2e9, 2.4e9, 201);
 * \endcode
 */
void PlanarS50244::set_freq_range(double start_freq, double stop_freq, int points) {
    this->start_freq = start_freq;
    this->stop_freq = stop_freq;
    this->points = points;

    freq_step = this->points <= 1 ? 0 : (this->stop_freq - this->start_freq) / (this->points - 1);

    send_wait_err(":SENSe:SWEep:POINts {}", this->points);

    send_wait_err(":SENSe:FREQuency:STARt {}", this->start_freq);
    send_wait_err(":SENSe:FREQuency:STOP {}", this->stop_freq);
}

/**
 * \brief Установка определённого значения частоты
 *
 * \param [in] freq Требуемая частота
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 * vna->set_freq(1.2e9);
 * \endcode
 */
void PlanarS50244::set_freq(double freq) {
    start_freq = freq;
    stop_freq = freq;
    points = 1;

    freq_step = 0;

    send_wait_err(":SENSe:SWEep:POINts {}", points);

    send_wait_err(":SENSe:FREQuency:STARt {}", start_freq);
    send_wait_err(":SENSe:FREQuency:STOP {}", stop_freq);
}

/**
 * \brief Отключает все порты
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_off();
 * \endcode
 */
void PlanarS50244::rf_off() {
    send_wait_err(":OUTPUT:STATE OFF");
}

/**
 * \brief Отключает выбранный порт
 *
 * \param [in] port Порт, который будет отключен
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_off(1);
 * \endcode
 */
void PlanarS50244::rf_off(int port) {
    send_wait_err(":OUTPUT:STATE OFF");
    send_wait_err(":CALC:PAR:SPOR {}", port);
}

/**
 * \brief Включает все порты
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_on();
 * \endcode
 */
void PlanarS50244::rf_on() {
    send_wait_err(":OUTPUT:STATE ON");
}

/**
 * \brief Включает выбранный порт
 *
 * \param [in] port Порт, который будет включен
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_oт(1);
 * \endcode
 */
void PlanarS50244::rf_on(int port) {
    send_wait_err(":CALC:PAR:SPOR {}", port);
    send_wait_err(":OUTPUT:STATE ON");
}

/**
 * \brief Перевод триггера в режим manual
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 * vna->trigger();
 * \endcode
 */
void PlanarS50244::trigger() {
    while (true) {
        std::string answer = send(":TRIGGER:STATUS?");

        if (answer == "HOLD") {
            send("INIT");
            send("TRIG:SING");

            do {
                answer = send(":TRIGGER:STATUS?");
            } while (answer != "HOLD");
        }

        break;
    }


}

/**
 * \brief Запуск измерения
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->trigger();
 * vna->init();
 * \endcode
 */
void PlanarS50244::init() {
    logger::log(LEVEL_WARN, "'init' not implemented for Planar S50244");
}

/**
 * \brief Сбор данных, полученных в результате измерения, для одного порта
 *
 * \param [in] trace_index Индекс трассы, которая соответствует требуемому порту
 *
 * \return Результат измерений одного порта
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new PlanarS50244("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 * vna->set_freq(1.2e9);
 *
 * vna->create_traces({2, 4, 5}, false);
 *
 * vna->trigger();
 * vna->init();
 *
 * iq_port_data_t port_data = vna->get_data(0);     // Получает данные для 2 порта
 * \endcode
 */
iq_port_data_t PlanarS50244::get_data(int trace_index) {
    iq_port_data_t iq_data{};

    std::string received_data = send(":CALCULATE:TRACE{}:DATA:SDATA?", trace_index + 1);
    std::vector<std::string> cached_data = string_utils::split(received_data, DATA_DELIMITER);

    for (int pos = 0; pos < cached_data.size(); pos += 2) {
        iq_data.emplace_back(cached_data[pos], cached_data[pos + 1]);
    }

    return iq_data;
}

void PlanarS50244::set_path(std::vector<int> path_list) {
    logger::log(LEVEL_WARN, "'set_path' not implemented for Planar S50244");
}
