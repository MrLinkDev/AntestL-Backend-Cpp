#ifndef ANTESTL_BACKEND_SOCKET_SERVER_HPP
#define ANTESTL_BACKEND_SOCKET_SERVER_HPP

#include <iostream>
#include <cstdio>
#include <cstring>
#include <utility>
#include <winsock2.h>
#include "../logger.hpp"

/// Стандартный IP адрес (127.0.0.1)
#define DEFAULT_ADDRESS         0x0100007F  // IP = 127.0.0.1
/// Стандартный порт
#define DEFAULT_PORT            5025

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
#define SOCKET_OK               0x03

/// Возвращаемый статус, если клиент был подключен к серверу
#define CLIENT_CONNECTED        0x04
/// Возвращаемый статус, если клиент не был подключен
#define CLIENT_NONE             0x05

/// Возвращаемый статус, если данные были успешно отправлены
#define DATA_SEND_OK            0x06
/// Возвращаемый статус, если данные не были отправлены
#define DATA_SEND_ERROR         0x07

/// Возвращаемый статус, если данные были успешно приняты
#define DATA_READ_OK            0x08
/// Возвращаемый статус, если данные не были приняты
#define DATA_READ_ERROR         0x09

typedef unsigned long address_t;
typedef unsigned short port_t;

class SocketServer {
    SOCKET server{}, client{};
    SOCKADDR_IN server_address{}, client_address{};

    address_t address;
    port_t port;

    std::string termination;

    WSADATA WSAData{};

public:
    SocketServer();
    SocketServer(port_t port);
    SocketServer(address_t address, port_t port);

    
};


#endif //ANTESTL_BACKEND_SOCKET_SERVER_HPP
