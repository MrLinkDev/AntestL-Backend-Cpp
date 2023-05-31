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

#define DEFAULT_ADDRESS         0x0100007F  // IP = 127.0.0.1
#define DEFAULT_PORT            5025

#define DEFAULT_BUFFER_SIZE     128

#define DEFAULT_TERMINATION     "\r\n"

#define SOCKET_NOT_CREATED      0x00
#define SOCKET_NOT_BOUND        0x01
#define SOCKET_NOT_LISTENING    0x02

#define SOCKET_OK               0x03

#define CLIENT_CONNECTED        0x04
#define CLIENT_NONE             0x05

#define DATA_SEND_OK            0x06
#define DATA_SEND_ERROR         0x07

#define DATA_READ_OK            0x08
#define DATA_READ_ERROR         0x09

class TcpServer {
    SOCKET server{}, client{};
    SOCKADDR_IN server_address{}, client_address{};

    unsigned long address;
    unsigned short port;

    std::string termination;

    WSADATA WSAData{};

    bool exist = false;

public:
    TcpServer() {
        this->address = DEFAULT_ADDRESS;
        this->port = DEFAULT_PORT;

        this->termination = DEFAULT_TERMINATION;
    }

    explicit TcpServer(unsigned short port) {
        this->address = DEFAULT_ADDRESS;
        this->port = port;

        this->termination = DEFAULT_TERMINATION;
    }

    TcpServer(unsigned long address, unsigned short port) {
        this->address = address;
        this->port = port;

        this->termination = DEFAULT_TERMINATION;
    }

    void set_termination(std::string term) {
        this->termination = std::move(term);
    }

    int create() {
        WSAStartup(MAKEWORD(2, 0), &WSAData);

        server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server == INVALID_SOCKET) {
            exist = false;

            logger::log(LEVEL_DEBUG, "Can't create socket!");
            return SOCKET_NOT_CREATED;
        }

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = address;
        server_address.sin_port = htons(port);

        int result = bind(server, (SOCKADDR *) &server_address, sizeof(server_address));
        if (result == SOCKET_ERROR) {
            exist = false;

            logger::log(LEVEL_ERROR, "Can't bind socket");
            return SOCKET_NOT_BOUND;
        }

        std::stringstream message;
        message << "Created socket (" << inet_ntoa(server_address.sin_addr) << ":" << port << ")";

        logger::log(LEVEL_INFO, message.str());
        return SOCKET_OK;
    }

    int wait_client() {
        if (listen(server, 0) == SOCKET_ERROR) {
            exist = false;

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

    bool is_exist() {
        return exist;
    }

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
