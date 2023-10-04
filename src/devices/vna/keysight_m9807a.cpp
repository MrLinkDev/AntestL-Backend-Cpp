/**
 * \file
 * \brief Файл исходного кода, в котором реализованы методы для класа KeysightM9807A
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#include "keysight_m9807a.hpp"
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
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * KeysightM9807A keysight_vna("TCPIP0::localhost::5025::SOCKET");
 * \endcode
 */
KeysightM9807A::KeysightM9807A(std::string device_address) : VnaDevice(std::move(device_address)) {}

/**
 * \brief Сброс настроек
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * vna->preset();
 * \endcode
 */
void KeysightM9807A::preset() {
    send(":SYSTEM:PRESET");
}

/**
  * \brief Полный сброс настроек
  *
  * **Пример**
  * \code
  * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
  * vna->full_preset();
  * \endcode
  */
void KeysightM9807A::full_preset() {
    send("*CLS");
    send("*RST");

    send(":SYSTEM:FPRESET");
    send(":DISPLAY:WINDOW1:STATE 1");
    send(":CALCULATE:PARAMETER:DELETE:ALL");

    send(":FORMAT:DATA ASCII,0");
}

/**
 * \brief Инициализация канала для измерений
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 * \endcode
 */
void KeysightM9807A::init_channel() {
    send_wait_err(R"(:CALCulate1:CUSTom:DEFine "TR0","Standard","S11")");

    send_wait_err(":SOURce:POWer:COUPle 1");

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:MODE OFF", port);
    }

    send_wait_err(":SOURce:POWer:COUPle 0");
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
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 * \endcode
 */
void KeysightM9807A::configure(int meas_type, float rbw, int source_port) {
    this->meas_type = meas_type;

    this->rbw = rbw;
    this->source_port = source_port;

    send_wait_err(":SENSE:ROSC:SOUR PXIBackplane");

    send_wait_err(":SENSe:SWEep:MODE HOLD");
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
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 *
 * vna->create_traces({2, 4, 5}, false);
 * \endcode
 */
void KeysightM9807A::create_traces(std::vector<int> port_list, bool external) {
    send_wait_err(":CALCulate:PARameter:DELete:ALL");

    std::string trace_name{};
    std::string trace_params{};

    int trace_arg = external ? 0 : source_port;

    for (int pos = 0; pos < port_list.size(); ++pos) {
        if (external) {
            char port_name = port_names[
                    array_utils::index(
                            port_numbers,
                            M9807A_PORT_COUNT,
                            port_list[pos]
                    )];

            char source_port_name = port_names[
                    array_utils::index(
                            port_numbers,
                            M9807A_PORT_COUNT,
                            port_list[source_port]
                    )];

            if (meas_type == MEAS_TRANSITION) {
                trace_name = std::format("{}/{}", port_name, source_port_name);
                trace_params = std::format("{},{}", trace_name, trace_arg);
            } else {
                trace_name = trace_params = std::format("S{}{}", port_list[pos], port_list[pos]);
            }
        } else {
            if (meas_type == MEAS_TRANSITION) {
                trace_name = trace_params = std::format("S{}{}", port_list[pos], source_port);
            } else {
                trace_name = trace_params = std::format("S{}{}", port_list[pos], port_list[pos]);
            }
        }

        send_wait_err(R"(:CALCulate1:CUSTom:DEFine "TR{}","Standard","{}")", port_list[pos], trace_params);
        send_wait_err(":DISPlay:WINDow:TRACe{}:FEED \"TR{}\"", port_list[pos], port_list[pos]);
    }
}

/**
 * \brief Установка требуемой мощности зондирующего порта
 *
 * \param [in] power Требуемое значение мощности
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->set_power(-10.0);
 * \endcode
 */
void KeysightM9807A::set_power(float power) {
    this->power = power;

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:LEVel:IMMediate:AMPLitude {},\"Port {}\"", port, this->power, port);
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
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 * vna->set_freq_range(1.2e9, 2.4e9, 201);
 * \endcode
 */
void KeysightM9807A::set_freq_range(double start_freq, double stop_freq, int points) {
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
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->full_preset();
 * vna->init_channel();
 *
 * vna->configure(MEAS_TRANSITION, 1e3, 1);
 * vna->set_freq(1.2e9);
 * \endcode
 */
void KeysightM9807A::set_freq(double freq) {
    start_freq = freq;
    stop_freq = freq;
    points = 1;

    freq_step = 0;

    send_wait_err(":SENSe:SWEep:POINts {}", points);

    send_wait_err(":SENSe:FREQuency:STARt {}", start_freq);
    send_wait_err(":SENSe:FREQuency:STOP {}", stop_freq);
}

/**
 * \brief Установка положений переключателей в определённые позиции
 *
 * \param [in] path_list
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * vna->set_path({1, 4, 5, 2});
 * \endcode
 */
void KeysightM9807A::set_path(std::vector<int> path_list) {
    for (int mod = 0; mod < M9807A_MODULE_COUNT; ++mod) {
        if (path_list[mod] == this->path_list[mod] || path_list[mod] == -1) {
            continue;
        }

        this->path_list[mod] = path_list[mod];
        send_wait_err("SENS:SWIT:M9157:MOD{}:SWIT:PATH STAT{}", mod + 1, this->path_list[mod]);
    }

    send_wait_err("INIT");
}

/**
 * \brief Запрос количества переключателей, подключенных к ВАЦ
 *
 * \return Количество переключателей
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * int count = vna->get_switch_module_count();
 * \endcode
 */
int KeysightM9807A::get_switch_module_count() {
    return M9807A_MODULE_COUNT;
}

/**
 * \brief Отключает все порты
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_off();
 * \endcode
 */
void KeysightM9807A::rf_off() {
    send_wait_err(":SOURce:POWer:COUPle 0");

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:MODE OFF", port);
    }
}

/**
 * \brief Отключает выбранный порт
 *
 * \param [in] port Порт, который будет отключен
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_off(1);
 * \endcode
 */
void KeysightM9807A::rf_off(int port) {
    send_wait_err(":SOURce:POWer:COUPle 0");
    send_wait_err(":SOURce:POWer{}:MODE OFF", port);
}

/**
 * \brief Включает все порты
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_on();
 * \endcode
 */
void KeysightM9807A::rf_on() {
    send_wait_err(":SOURce:POWer:COUPle 0");

    for (int port = 1; port < M9807A_PORT_COUNT + 1; ++port) {
        send_wait_err(":SOURce:POWer{}:MODE ON", port);
    }
}

/**
 * \brief Включает выбранный порт
 *
 * \param [in] port Порт, который будет включен
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * vna->rf_oт(1);
 * \endcode
 */
void KeysightM9807A::rf_on(int port) {
    send_wait_err(":SOURce:POWer:COUPle 0");
    send_wait_err(":SOURce:POWer{}:MODE ON", port);
}

/**
 * \brief Перевод триггера в режим manual
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 * vna->trigger();
 * \endcode
 */
void KeysightM9807A::trigger() {
    send_wait_err(":TRIGger:SEQuence:SOURce MANual");
    send_wait_err(":TRIGger:SEQuence:SCOPe CURRent");

    send_wait_err("TRIG:SCOP ALL");
}

/**
 * \brief Запуск измерения
 *
 * **Пример**
 * \code
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
 *
 * vna->trigger();
 * vna->init();
 * \endcode
 */
void KeysightM9807A::init() {
    send_wait_err("INIT:IMM");
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
 * VnaDevice *vna = new KeysightM9807A("TCPIP0::localhost::5025::SOCKET");
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
iq_port_data_t KeysightM9807A::get_data(int trace_index) {
    iq_port_data_t iq_data{};

    std::string received_data = send(":CALCULATE:MEASURE{}:DATA:SDATA?", trace_index + 1);
    std::vector<std::string> cached_data = string_utils::split(received_data, DATA_DELIMITER);

    for (int pos = 0; pos < cached_data.size(); pos += 2) {
        iq_data.emplace_back(cached_data[pos], cached_data[pos + 1]);
    }

    return iq_data;
}