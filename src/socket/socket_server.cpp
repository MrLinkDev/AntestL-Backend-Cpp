#include "socket_server.hpp"

SocketServer::SocketServer() {
    this->address = DEFAULT_ADDRESS;
    this->port = DEFAULT_PORT;

    this->termination = DEFAULT_SOCKET_TERM;
}

SocketServer::SocketServer(port_t port) {
    this->address = DEFAULT_ADDRESS;
    this->port = port;

    this->termination = DEFAULT_SOCKET_TERM;
}

SocketServer::SocketServer(address_t address, port_t port) {
    this->address = address;
    this->port = port;

    this->termination = DEFAULT_SOCKET_TERM;
}

void SocketServer::set_port(port_t port) {
    this->port = port;
}

void SocketServer::set_termination(std::string termination) {
    this->termination = std::move(termination);
}

int SocketServer::create() {
    WSAStartup(MAKEWORD(2, 0), &WSAData);

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) {
        logger::log(LEVEL_ERROR, "SOCKET({}): Can't create socket!", port);
        return SOCKET_NOT_CREATED;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = address;
    server_address.sin_port = htons(port);

    int result = bind(server, (SOCKADDR *) &server_address, sizeof(server_address));
    if (result == SOCKET_ERROR) {
        logger::log(LEVEL_ERROR, "SOCKET({}): Can't bind socket", port);
        return SOCKET_NOT_BOUND;
    }

    logger::log(LEVEL_DEBUG, "SOCKET({}): Created", port);
    return SOCKET_CREATED;
}

int SocketServer::wait_client() {
    if (listen(server, 0) == SOCKET_ERROR) {
        logger::log(LEVEL_ERROR, "SOCKET({}): Can't start listening", port);
        return SOCKET_NOT_LISTENING;
    }

    logger::log(LEVEL_INFO, "SOCKET({}): Listening...", port);

    int client_address_size = sizeof(client_address);
    if ((client = accept(server, (SOCKADDR*)&client_address, &client_address_size)) != INVALID_SOCKET) {
        logger::log(LEVEL_INFO, "SOCKET({}): Connected client with address {}", port, inet_ntoa(client_address.sin_addr));
        return CLIENT_CONNECTED;
    }

    logger::log(LEVEL_ERROR, "SOCKET({}): No clients...", port);
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

                logger::log(LEVEL_TRACE, "SOCKET({}): Got data from client = {}", port, data);

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
    logger::log(LEVEL_TRACE, "SOCKET({}): Sending data to client = {}0", data);

    data.append(termination);

    if (send(client, data.c_str(), data.length(), 0) == SOCKET_ERROR) {
        logger::log(LEVEL_ERROR, "Can't send message!");
        return DATA_SEND_ERROR;
    }

    return DATA_SEND_OK;
}
