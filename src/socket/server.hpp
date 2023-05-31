/**
 * \file
 * \brief Заголовочный файл, в котором реализуется TCP сервер
 *
 * Объект сервера содержит в себе методы для обмена данными между сервером и клиентом
 *
 * \author Александр Горбунов
 * \date 30 мая 2023
 */

#ifndef ANTESTL_BACKEND_SERVER_HPP
#define ANTESTL_BACKEND_SERVER_HPP

#include <iostream>
#include <cstdio>
#include <cstring>
#include <utility>
#include <winsock2.h>
#include "../utils/logger.hpp"

/// Стандартный IP адрес (127.0.0.1)
#define DEFAULT_ADDRESS         0x0100007F  // IP = 127.0.0.1
/// Стандартный порт
#define DEFAULT_PORT            5025

/// Стандартный размер буфера
#define DEFAULT_BUFFER_SIZE     128

/// Стандартная последовательность конца посылки
#define DEFAULT_TERMINATION     "\r\n"

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

/**
 * \brief TCP-сервер для обмена данными с подключенными клиентами.
 *
 * Объект содержит в себе методы для создания TCP-сервера, ожидания клиента,
 * отправки и получения данных.
 */
class TcpServer {
    SOCKET server{}, client{};
    SOCKADDR_IN server_address{}, client_address{};

    unsigned long address;
    unsigned short port;

    std::string termination;

    WSADATA WSAData{};

public:
    /**
     * \brief Стандартный конструктор
     *
     * Создаёт объект со следующими параметрами:
     * - адрес: 127.0.0.1
     * - порт: 5025
     * - конец посылки: \\r\\n
     *
     * **Пример**
     * \code
     * TcpServer server();
     * \endcode
     */
    TcpServer() {
        this->address = DEFAULT_ADDRESS;
        this->port = DEFAULT_PORT;

        this->termination = DEFAULT_TERMINATION;
    }

    /**
     * \brief Конструктор, позволяющий задать порт
     *
     * Создаёт объект со следующими параметрами:
     * - адрес: 127.0.0.1
     * - порт: переданный в качестве аргумента
     * - конец посылки: \\r\\n
     *
     * **Пример**
     * \code
     * TcpServer server(5025);
     * \endcode
     */
    explicit TcpServer(unsigned short port) {
        this->address = DEFAULT_ADDRESS;
        this->port = port;

        this->termination = DEFAULT_TERMINATION;
    }

    /**
    * \brief Конструктор, позволяющий задать адрес и порт
    *
    * Создаёт объект со следующими параметрами:
    * - адрес: переданный в качестве аргумента
    * - порт: переданный в качестве аргумента
    * - конец посылки: \\r\\n
    *
    * **Пример**
    * \code
    * TcpServer server("192.168.0.1", 5025);
    * \endcode
    */
    TcpServer(unsigned long address, unsigned short port) {
        this->address = address;
        this->port = port;

        this->termination = DEFAULT_TERMINATION;
    }

    /**
     * \brief Функция для изменения конца посылки
     *
     * \param [in] term - Новый конец посылки
     *
     * **Пример**
     * \code
     * server.set_termination("\n");
     * \endcode
     */
    void set_termination(std::string term) {
        this->termination = std::move(term);
    }

    /**
     * \brief Функция для создания сокета.
     *
     * Создаёт сокет с параметрами, которые были переданы в конструкторе.
     *
     * \return Возвращает один из следующих возможных статусов:
     * - SOCKET_NOT_CREATED - сокет не был создан
     * - SOCKET_NOT_BOUND - сокет не был привязан
     * - SOCKET_OK - сокет создан и привязан
     *
     * **Пример**
     * \code
     * int status = server.create();
     *
     * if (status != SOCKET_OK) {
     *     printf("Error");
     * } else {
     *     printf("Created");
     * }
     * \endcode
     */
    int create() {
        WSAStartup(MAKEWORD(2, 0), &WSAData);

        server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server == INVALID_SOCKET) {
            logger::log(LEVEL_DEBUG, "Can't create socket!");
            return SOCKET_NOT_CREATED;
        }

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = address;
        server_address.sin_port = htons(port);

        int result = bind(server, (SOCKADDR *) &server_address, sizeof(server_address));
        if (result == SOCKET_ERROR) {
            logger::log(LEVEL_ERROR, "Can't bind socket");
            return SOCKET_NOT_BOUND;
        }

        std::stringstream message;
        message << "Created socket (" << inet_ntoa(server_address.sin_addr) << ":" << port << ")";

        logger::log(LEVEL_INFO, message.str());
        return SOCKET_OK;
    }

    /**
     * \brief Функция, которая запускает прослушивание порта и ожидает клиента.
     * \warning Данную функцию следует вызывать только после вызова метода create().
     *
     * \return Возвращает один из следующих возможных статусов:
     * - SOCKET_NOT_LISTENING - не может быть запущено прослушивание
     * - CLIENT_CONNECTED - клиент был подключен
     * - CLIENT_NONE - нет клиента
     *
     * **Пример**
     * \code
     * TcpServer server();
     * int status = server.create();
     *
     * if (status != SOCKET_OK) {
     *     printf("Error");
     *     return
     * }
     *
     * status = server.wait_client();
     * if (status == CLIENT_CONNECTED) {
     *     printf("Client connected");
     * }
     * \endcode
     */
    int wait_client() {
        if (listen(server, 0) == SOCKET_ERROR) {
            logger::log(LEVEL_ERROR, "Socket can't listen");
            return SOCKET_NOT_LISTENING;
        }

        logger::log(LEVEL_INFO, "Listening...");

        int client_address_size = sizeof(client_address);
        if ((client = accept(server, (SOCKADDR*)&client_address, &client_address_size)) != INVALID_SOCKET) {

            std::stringstream message;
            message << "Connected client with address " << inet_ntoa(client_address.sin_addr);
            logger::log(LEVEL_INFO, message.str());

            return CLIENT_CONNECTED;
        }

        logger::log(LEVEL_ERROR, "No clients...");
        return CLIENT_NONE;
    }

    /**
     * \brief Функция, осуществляющая чтение данных от клиента.
     *
     * Данные записываются в переданный буфер. Буфер заполняется до тех пор, пока не
     * встретятся символы, означающие конец посылки.
     *
     * \param [out] data - Буфер для записи принятых данных.
     * \return Если данные приняты без ошибок, то возвращает DATA_READ_OK.
     * В противном случае - DATA_READ_ERROR.
     *
     * **Пример**
     * \code
     * TcpServer server();
     * int status = server.create();
     *
     * if (status != SOCKET_OK) {
     *     printf("Error");
     *     return
     * }
     *
     * status = server.wait_client();
     * if (status == CLIENT_CONNECTED) {
     *     string data{};
     *     server.read_data(data);
     *
     *     printf("%s", data);
     * }
     * \endcode
     */
    int read_data(std::string &data) {
        char buffer[DEFAULT_BUFFER_SIZE] = { 0 };
        bool finish = false;

        while (true) {
            if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
                return DATA_READ_ERROR;
            }

            for (int pos = 0; pos < DEFAULT_BUFFER_SIZE; ++pos) {
                if (buffer[pos] == termination[0]) {
                    finish = true;
                } else if (buffer[pos] == termination[1] && finish) {
                    memset(buffer, 0, sizeof(buffer));

                    logger::log(LEVEL_INFO, "Data was read from client");
                    logger::log(LEVEL_DEBUG, data);

                    return DATA_READ_OK;
                } else if (buffer[pos] == '\0') {
                    continue;
                } else {
                    if (finish) {
                        finish = false;
                    }

                    data += buffer[pos];
                }
            }

            memset(buffer, 0, sizeof(buffer));
        }
    }

    /**
     * \brief Функция, осуществляющая отправку данных клиенту.
     *
     * К передаваемым данным добавляется конец посылки, после чего они отправляются.
     *
     * \param [in] data - Передаваемые данные.
     * \return Если данные отправлены без ошибок, то возвращает DATA_SEND_OK.
     * В противном случае - DATA_SEND_ERROR.
     *
     * **Пример**
     * \code
     * TcpServer server();
     * int status = server.create();
     *
     * if (status != SOCKET_OK) {
     *     printf("Error");
     *     return
     * }
     *
     * status = server.wait_client();
     * if (status == CLIENT_CONNECTED) {
     *     string data = "Test command";
     *     server.send_data(data);
     * }
     * \endcode
     */
    int send_data(std::string data) {
        logger::log(LEVEL_INFO, "Sending data to client");
        logger::log(LEVEL_DEBUG, data);

        data.append(termination);

        if (send(client, data.c_str(), data.length(), 0) == SOCKET_ERROR) {
            logger::log(LEVEL_ERROR, "Can't send message!");
            return DATA_SEND_ERROR;
        }

        logger::log(LEVEL_INFO, "Data sent to client");

        return DATA_SEND_OK;
    }
};

typedef TcpServer tcp_server;


#endif //ANTESTL_BACKEND_SERVER_HPP
