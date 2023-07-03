/**
 * \file
 * \brief Заголовочный файл, содержащий класс antestl_exsceptions,
 * сообщения исключений и коды исключений
 *
 * \authors Александр Горбунов
 * \date 3 июля 2023 года
 */

#ifndef ANTESTL_BACKEND_EXCEPTIONS_HPP
#define ANTESTL_BACKEND_EXCEPTIONS_HPP

/// Сообщение исключения при невозможности выполнения команды "*OPC?"
#define OPC_ERROR_MSG           "Unable to execute OPC"
/// Код исключения при невозможности выполнения команды "*OPC?"
#define OPC_ERROR_CODE          0xE000

/// Сообщение исключения при ошибке получения данных от устройства
#define READ_ERROR_MSG          "Can't read data from device"
/// Код исключения при ошибке получения данных от устройства
#define READ_ERROR_CODE         0xE100

/// Сообщение исключения при ошибке отправки данных на устройство
#define WRITE_ERROR_MSG         "Can't write data into device"
/// Код исключения при ошибке отправки данных на устройство
#define WRITE_ERROR_CODE        0xE200

/// Сообщение исключения в случае, когда ошибка возникла на устройстве
#define DEVICE_ERROR_MSG        "Caught error from device"
/// Код исключения в случае, когда ошибка возникла на устройстве
#define DEVICE_ERROR_CODE       0xE300

/// Сообщение исключения в случае, когда не получилось подключиться к устройству
#define NO_CONNECTION_MSG       "No connection with device"
/// Код исключения в случае, когда не получилось подключиться к устройству
#define NO_CONNECTION_CODE      0xEF00

/**
 * \brief Класс исключения, содержащий в себе сообщение код исключения
 */
class antestl_exception : public std::exception {
    /// Сообщение исключения
    std::string message;
    /// Код исключения
    int code;

public:
    /**
     * \brief Конструктор объекта
     *
     * Переданные в конструктор параметры присваиваются параметрам созданного объекта
     *
     * \param [in] message Сообщение исключения
     * \param [in] code Код исключения
     */
    antestl_exception(std::string message, int code) {
        this->message = std::move(message);
        this->code = code;
    }

    /**
     * \brief Функция, возвращающая сообщение исключения
     *
     * \return Сообщение исключения
     */
    std::string what() {
        return message;
    }

    /**
     * \brief Функция, возвращающая код исключения
     *
     * \return Код исключения
     */
    int error_code() const {
        return code;
    }
};

#endif //ANTESTL_BACKEND_EXCEPTIONS_HPP
