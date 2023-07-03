/**
 * \file
 * \brief Заголовочный файл, в котором объявляются класс SocketServer и необходимые константы
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_SOCKET_SERVER_HPP
#define ANTESTL_BACKEND_SOCKET_SERVER_HPP

#include <winsock2.h>
#include "../utils/logger.hpp"

/// Стандартный IP адрес (127.0.0.1)
#define DEFAULT_ADDRESS         0x0100007F  // IP = 127.0.0.1
/// Стандартный порт
#define DEFAULT_PORT            5025

/// Стандартный тег
#define DEFAULT_TAG             "SOCKET"

/// Стандартный размер буфера
#define DEFAULT_BUFFER_SIZE     128

/// Стандартная последовательность конца посылки
#define DEFAULT_SOCKET_TERM     "\r\n"

/// Возвращаемый статус, если сокет не был создан
#define SOCKET_NOT_CREATED      0x00
/// Возвращаемый статус, если сокет не был привязан
#define SOCKET_NOT_BOUND        0x01
/// Возвращаемый статус, если не запущено прослушивание
#define SOCKET_NOT_LISTENING    0x02

/// Возвращаемый статус, если сокет был успешно создан
#define SOCKET_CREATED          0x03

/// Возвращаемый статус, если клиент был подключен к серверу
#define CLIENT_CONNECTED        0x04
/// Возвращаемый статус, если клиент не был подключен
#define CLIENT_NONE             0x05

/// Возвращаемый статус, если данные были успешно отправлены
#define DATA_SEND_OK            0x06
/// Возвращаемый статус, если данные не были отправлены
#define DATA_SEND_ERROR         0x07

/// Определение типа данных для IP-адреса
typedef unsigned long address_t;
/// Определение типа данных для порта
typedef unsigned short port_t;

/**
 * \class
 * \brief Класс SocketServer, в котором имеется набор методов для установки соединения и обмена данными с клиентом
 */
class SocketServer {
    /// Объекты сокета для сервера и клиента
    SOCKET server{}, client{};
    /// Структуры адресов сервера и клиента
    SOCKADDR_IN server_address{}, client_address{};

    /// Адрес сервера
    address_t address;
    /// Порт сервера
    port_t port;

    /// Тег сервера, необходимый для логирования действий
    std::string tag;

    /// Последовательность символов, которой оканчивается каждая посылка
    std::string termination;

    /// Структура, содержащая сведения о реализации сокета для ОС Windows
    WSADATA WSAData{};

public:
    SocketServer();
    SocketServer(port_t port, std::string tag = DEFAULT_TAG);
    SocketServer(address_t address, port_t port, std::string tag = DEFAULT_TAG);

    void set_port(port_t port);

    int create();
    int wait_client();

    std::string read_data();
    int send_data(std::string data);

    void close();
};


#endif //ANTESTL_BACKEND_SOCKET_SERVER_HPP
