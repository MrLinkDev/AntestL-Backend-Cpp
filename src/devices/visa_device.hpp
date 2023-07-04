/**
 * \file
 * \brief Заголовочный файл, в котором определены константы и класс VisaDevice
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_VISA_DEVICE_HPP
#define ANTESTL_BACKEND_VISA_DEVICE_HPP

#include <format>
#include <cstring>

#include "visa.h"
#include "../utils/logger.hpp"

/// Размер буфера для данных, которые приходят от прибора
#define BUFFER_SIZE             128

/// Стандартный таймаут команды
#define DEFAULT_TIMEOUT         1000000
/// Стандартный символ окончания посылки
#define DEFAULT_VISA_TERM       '\n'

/// Код, возвращаемый при успешно выполненной операции
#define SUCCESS                 0x00
/// Код, возвращаемый при неудачном выполнении операции
#define FAILURE                 0x01

/// Код, возвращаемый, если на приборе не возникло ошибок
#define NO_ERRORS               0x02
/// Код, возвращаемый, если на приборе возникли ошибки
#define ERRORS                  0x03

/// Код, возвращаемый функцией opc(), если прибор завершил выполнение действий
#define OPC_PASS                0x04
/// Код, возвращаемый функцией opc(), если прибор не завершил выполнение действий
#define OPC_WAIT                0x05

/// Команда для запроса информации о приборе
#define CMD_IDN                 "*IDN?"
/// Команда для проверки статуса выполнения действий прибором
#define CMD_OPC                 "*OPC?"
/// Команда для запроса ошибок, возникших на приборе
#define CMD_ERR                 "SYSTEM:ERROR?"

/// Стандартный ответ прибора, если на нём не возникло ошибок
#define NO_ERROR_STR            "+0,\"No error\""

/// Стандартный ответ прибора компании Keysight на команду "*OPC?", если все действия завершены
#define OPC_PASS_STR_KEYSIGHT   "+1"
/// Стандартный ответ прибора компании Planar на команду "*OPC?", если все действия завершены
#define OPC_PASS_STR_PLANAR     "1"

/**
 * \brief Структура конфигурации прибора
 *
 * Структура содержит в себе минимальный набор параметров для создания подключения с прибором
 */
struct visa_config {
    /// Адрес устройства
    std::string address{};

    /// Таймаут выполнения команды. По-умолчанию таймаут = DEFAULT_TIMEOUT.
    int timeout = DEFAULT_TIMEOUT;
    /// Символ, которым оканчивается посылка. По-умолчанию символ = DEFAULT_VISA_TERM.
    char termination = DEFAULT_VISA_TERM;
};

/**
 * \brief Класс, в котором реализованы методы для управления устройством с помощью API VISA.
 */
class VisaDevice {
    /// Конфигурация устройства для подключения
    visa_config device_config{};

    /// Идентификатор менеджера ресурсов
    ViSession resource_manager{};
    /// Идентификатор устройства
    ViSession device{};

    /// Статус выполнения операции
    ViStatus status{};

    ViPUInt32 ret_count{};

    int write(std::string command);
    std::string read();

    std::string query(std::string command);

protected:
    /// Переменная, которая показывает, подключен ли прибор или нет
    bool connected = false;

public:
    VisaDevice() = default;
    explicit VisaDevice(std::string device_address);

    ~VisaDevice();

    virtual void connect();

    bool is_connected() const;
    void clear() const;

    std::string idn();
    int opc();
    int err();

    void wait();

    std::string send(std::string command, bool read_data = false);
    std::string send_wait(std::string command);
    std::string send_err(std::string command);
    std::string send_wait_err(std::string command);

    /**
     * \brief Отправка данных на прибор и чтение ответа от прибора.
     *
     * Данный метод позволяет сформировать команду, которая будет отправлена
     * на прибор, с помощью строки форматирования и набора аргументов для неё.
     * Если строка заканчивается символом '?', то ожидается ответ от прибора.
     *
     * \param [in] fmt Строка форматирования
     * \param [in] args Аргументы для строки форматирования
     *
     * \return Если ожидался ответ от прибора, то возвращает полученные данные.
     * В противном случае, возвращает пустую строку
     *
     * **Пример**
     * \code
     * VisaDevice vna("TCPIP0::localhost::5025::SOCKET");
     * vna.connect();
     *
     * int points = 201;
     *
     * if (vna.is_connected()) {
     *     vna.send(":SENSe:SWEep:POINts {}", points);
     *     std::string answer = vna.send(":SENSe:SWEep:POINts?");
     *
     *     std::cout << answer << std::endl;        // Будет выведено число 201
     * }
     * \endcode
     */
    template <typename... T>
    std::string send(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        data = send(command);

        return data;
    }

    /**
     * \brief Отправка данных на прибор и чтение ответа от прибора. Также,
     * ожидается завершение выполнения действий прибором.
     *
     * Данный метод позволяет сформировать команду, которая будет отправлена
     * на прибор, с помощью строки форматирования и набора аргументов для неё.
     * Метод ожидает завершение выполнения команды. Если строка заканчивается
     * символом '?', то метод ожидает ответ от прибора. В противном случае
     * возвращается пустая строка.
     *
     * \param [in] fmt Строка форматирования
     * \param [in] args Аргументы для строки форматирования
     *
     * \return Если ожидался ответ от прибора, то возвращает полученные данные.
     * В противном случае, возвращает пустую строку
     *
     * **Пример**
     * \code
     * VisaDevice vna("TCPIP0::localhost::5025::SOCKET");
     * vna.connect();
     *
     * int points = 2001;
     *
     * if (vna.is_connected()) {
     *     vna.send(":SENSe:SWEep:POINts {}", points);
     *     vna.send_wait("INIT");
     *
     *     std::string data = vna.send(":CALCULATE:MEASURE:DATA:SDATA?");
     * }
     * \endcode
     */
    template <typename... T>
    std::string send_wait(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        data = send_wait(command);

        return data;
    }

    /**
     * \brief Отправка данных на прибор и чтение ответа от прибора.
     * Также, проверяется наличие ошибок на приборе.
     *
     * Данный метод позволяет сформировать команду, которая будет
     * отправлена на прибор, с помощью строки форматирования и набора
     * аргументов для неё. Метод проверяет наличие ошибок на приборе
     * после выполнения команды. Если строка заканчивается символом '?',
     * то функция ожидает ответ от прибора.
     *
     * \param [in] fmt Строка форматирования
     * \param [in] args Аргументы для строки форматирования
     *
     * \return Если ожидался ответ от прибора, то возвращает полученные
     * данные. В противном случае, возвращает пустую строку
     *
     * **Пример**
     * \code
     * VisaDevice vna("TCPIP0::localhost::5025::SOCKET");
     * vna.connect();
     *
     * if (vna.is_connected()) {
     *     // Метод бросит исключение antestl_exception с кодом DEVICE_ERROR_CODE
     *     vna.send_err("IDN");
     * }
     * \endcode
     */
    template <typename... T>
    std::string send_err(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        data = send_err(command);

        return data;
    }

    /**
     * \brief Отправка данных на прибор и чтение ответа от прибора. Также,
     * ожидается завершение выполнения действий прибором и проверяется наличие
     * ошибок на приборе.
     *
     * Данный метод позволяет сформировать команду, которая будет отправлена
     * на прибор, с помощью строки форматирования и набора аргументов для неё.
     * Метод ожидает завершение выполнения команды проверяет наличие ошибок
     * на приборе после выполнения команды. Если строка заканчивается
     * символом '?', то метод ожидает ответ от прибора. В противном случае
     * возвращается пустая строка.
     *
     * \param [in] fmt Строка форматирования
     * \param [in] args Аргументы для строки форматирования
     *
     * \return Если ожидался ответ от прибора, то возвращает полученные данные.
     * В противном случае, возвращает пустую строку
     *
     * **Пример**
     * \code
     * VisaDevice vna("TCPIP0::localhost::5025::SOCKET");
     * vna.connect();
     *
     * if (vna.is_connected()) {
     *     vna.send_wait_err("SYSTEM:FPRESET");
     * }
     * \endcode
     */
    template <typename... T>
    std::string send_wait_err(const std::string &fmt, T &&...args) {
        std::string command = std::vformat(fmt, std::make_format_args(args...));
        std::string data{};

        data = send_wait_err(command);

        return data;
    }
};

typedef VisaDevice visa_device_t;

#endif //ANTESTL_BACKEND_VISA_DEVICE_HPP
