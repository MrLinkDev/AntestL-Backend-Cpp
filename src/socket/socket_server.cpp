#include "socket_server.hpp"

/**
 * \brief Стандартный конструктор.
 *
 * Создаётся объект со стандартными параметрами:
 * - address = 127.0.0.1
 * - port = 5025
 * - tag = "SOCKET"
 *
 * **Пример**
 * \code
 * SocketServer socket_s();
 * \endcode
 */
SocketServer::SocketServer() {
    this->address = DEFAULT_ADDRESS;
    this->port = DEFAULT_PORT;

    this->termination = DEFAULT_SOCKET_TERM;
}

/**
 * \brief Конструктор, в который передаётся порт и тег
 *
 * \param [in] port Порт сокета
 * \param [in] tag Тег сокета. Может быть пустым, тогда тег будет стандартным.
 *
 * **Пример**
 * \code
 * SocketServer socket_s(5555, "TEST SOCKET");
 * \endcode
 */
SocketServer::SocketServer(port_t port, std::string tag) {
    this->address = DEFAULT_ADDRESS;
    this->port = port;

    this->tag = std::move(tag);

    this->termination = DEFAULT_SOCKET_TERM;
}

/**
 * \brief Конструктор, в который передаётся порт и тег
 *
 * \param [in] port Порт сокета
 * \param [in] tag Тег сокета. Может быть пустым, тогда тег будет стандартным.
 *
 * **Пример***
 * \code
 * SocketServer socket_s(5555, "TEST SOCKET");
 * \endcode
 */
SocketServer::SocketServer(address_t address, port_t port, std::string tag) {
    this->address = address;
    this->port = port;

    this->tag = std::move(tag);

    this->termination = DEFAULT_SOCKET_TERM;
}

/**
 * \brief Меняет порт на требуемый
 *
 * \param [in] port Требуемый порт
 */
void SocketServer::set_port(port_t port) {
    this->port = port;
}

/**
 * \brief Функция, инициализирующая сокет
 *
 * \return В зависимости от результата, возвращает соответствующий код результата.
 *
 * **Пример**
 * \code
 *
 * \endcode
 */
int SocketServer::create() {
    WSAStartup(MAKEWORD(2, 0), &WSAData);

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) {
        logger::log(LEVEL_ERROR, "{} ({}): Can't create socket!", tag, port);
        return SOCKET_NOT_CREATED;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = address;
    server_address.sin_port = htons(port);

    int result = bind(server, (SOCKADDR *) &server_address, sizeof(server_address));
    if (result == SOCKET_ERROR) {
        logger::log(LEVEL_ERROR, "{} ({}): Can't bind socket", tag, port);
        return SOCKET_NOT_BOUND;
    }

    logger::log(LEVEL_DEBUG, "{} ({}): Created", tag, port);
    return SOCKET_CREATED;
}

/**
 * \brief Функция, ожидающая подключение клиента
 *
 * \return В зависимости от результата, возвращает соответствующий код результата.
 */
int SocketServer::wait_client() {
    if (listen(server, 0) == SOCKET_ERROR) {
        logger::log(LEVEL_ERROR, "{} ({}): Can't start listening", tag, port);
        return SOCKET_NOT_LISTENING;
    }

    logger::log(LEVEL_INFO, "{} ({}): Listening...", tag, port);

    int client_address_size = sizeof(client_address);
    if ((client = accept(server, (SOCKADDR*)&client_address, &client_address_size)) != INVALID_SOCKET) {
        logger::log(LEVEL_INFO, "{} ({}): Connected client with address {}", tag, port, inet_ntoa(client_address.sin_addr));
        return CLIENT_CONNECTED;
    }

    logger::log(LEVEL_ERROR, "{} ({}): No clients...", tag, port);
    return CLIENT_NONE;
}

std::string SocketServer::read_data() {
    std::string data{};

    char buffer[DEFAULT_BUFFER_SIZE] = { 0 };
    bool finish = false;

    while (true) {
        if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            return std::string{};
        }

        for (int pos = 0; pos < DEFAULT_BUFFER_SIZE; ++pos) {
            if (buffer[pos] == termination[0]) {
                finish = true;
            } else if (buffer[pos] == termination[1] && finish) {
                memset(buffer, 0, sizeof(buffer));

                logger::log(LEVEL_TRACE, "{} ({}): Got data from client = {}", tag, port, data);

                return data;
            } else if (buffer[pos] == '\0') {
                continue;
            } else {
                if (finish) {
                    finish = false;
                }

                data.push_back(buffer[pos]);
            }
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

int SocketServer::send_data(std::string data) {
    logger::log(LEVEL_TRACE, "{} ({}): Sending data to client = {}", tag, port, data);

    data.append(termination);

    if (send(client, data.c_str(), data.length(), 0) == SOCKET_ERROR) {
        logger::log(LEVEL_ERROR, "{} ({}): Can't send data!", tag, port);
        return DATA_SEND_ERROR;
    }

    return DATA_SEND_OK;
}

void SocketServer::close() {
    closesocket(client);

    if (closesocket(server) == SOCKET_ERROR) {
        logger::log(LEVEL_ERROR, "{} ({}): Can't close socket", tag, port);
    }
}
