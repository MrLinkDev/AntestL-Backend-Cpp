#include <iostream>
#include <visa.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <winsock2.h>
#include "socket/server.hpp"
#include "utils/visa_device.hpp"

#define MAX_CNT 200

using namespace std;

WSADATA WSAData; //Данные
SOCKET server, client; //Сокеты сервера и клиента
SOCKADDR_IN serverAddr, clientAddr; //Адреса сокетов

DWORD WINAPI serverReceive(LPVOID lpParam) { //Получение данных от клиента
    char buffer[1024] = { 0 }; //Буфер для данных
    SOCKET client = *(SOCKET*)lpParam; //Сокет для клиента
    while (true) { //Цикл работы сервера
        if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            //Если не удалось получить данные буфера, сообщить об ошибке и выйти
            cout << "recv function failed with error " << WSAGetLastError() << endl;
            return -1;
        }
        if (strcmp(buffer, "exit\n") == 0) { //Если клиент отсоединился
            cout << "Client Disconnected." << endl;
            break;
        }
        cout << "Client: " << buffer << endl; //Иначе вывести сообщение от клиента из буфера
        memset(buffer, 0, sizeof(buffer)); //Очистить буфер
    }
    return 1;
}

DWORD WINAPI serverSend(LPVOID lpParam) { //Отправка данных клиенту
    char buffer[1024] = { 0 };
    SOCKET client = *(SOCKET*)lpParam;
    while (true) {
        fgets(buffer, 1024, stdin);
        if (send(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            cout << "send failed with error " << WSAGetLastError() << endl;
            return -1;
        }
        if (strcmp(buffer, "exit\n") == 0) {
            cout << "Thank you for using the application" << endl;
            break;
        }
    }
    return 1;
}

int prepare() {
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0); //Создали сервер
    if (server == INVALID_SOCKET) {
        cout << "Socket creation failed with error:" << WSAGetLastError() << endl;
        return -1;
    }
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);
    if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind function failed with error: " << WSAGetLastError() << endl;
        return -1;
    }
}

int main(int argc, char* argv[]) {
   /** ViStatus status;
    ViSession defaultRM, instr;
    ViPUInt32 retCount;
    ViPBuf buffer[MAX_CNT];

    status = viOpenDefaultRM(&defaultRM);

    if (status < VI_SUCCESS)
        return -1;

    status = viOpen(
            defaultRM,
            "TCPIP0::K-N9020B-11111::5025::SOCKET",
            VI_NULL,
            VI_NULL,
            &instr);

    status = viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);

    status = viWrite(instr, reinterpret_cast<ViConstBuf>("*IDN?\n"), 6, reinterpret_cast<ViPUInt32>(&retCount));
    status = viRead(instr, reinterpret_cast<ViPBuf>(buffer), MAX_CNT, reinterpret_cast<ViPUInt32>(&retCount));

    printf("%s", buffer);

    status = viClose(instr);
    status = viClose(defaultRM);*/

    /** WSADATA WSAData; //Данные
    SOCKET server, client; //Сокеты сервера и клиента
    SOCKADDR_IN serverAddr, clientAddr; //Адреса сокетов
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0); //Создали сервер
    if (server == INVALID_SOCKET) {
        cout << "Socket creation failed with error:" << WSAGetLastError() << endl;
        return -1;
    }
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);
    if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind function failed with error: " << WSAGetLastError() << endl;
        return -1;
    }

    if (listen(server, 0) == SOCKET_ERROR) { //Если не удалось получить запрос
        cout << "Listen function failed with error:" << WSAGetLastError() << endl;
        return -1;
    }
    cout << "Listening for incoming connections...." << endl;

    char buffer[1024]; //Создать буфер для данных
    int clientAddrSize = sizeof(clientAddr); //Инициализировать адерс клиента
    if ((client = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
        //Если соединение установлено
        cout << "Client connected!" << endl;
        cout << "Now you can use our live chat application. " << "Enter \"exit\" to disconnect" << endl;

        DWORD tid; //Идентификатор
        HANDLE t1 = CreateThread(NULL, 0, serverReceive, &client, 0, &tid); //Создание потока для получения данных
        if (t1 == NULL) { //Ошибка создания потока
            cout << "Thread Creation Error: " << WSAGetLastError() << endl;
        }
        HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid); //Создание потока для отправки данных
        if (t2 == NULL) {
            cout << "Thread Creation Error: " << WSAGetLastError() << endl;
        }

        WaitForSingleObject(t1, INFINITE);
        WaitForSingleObject(t2, INFINITE);

        closesocket(client); //Закрыть сокет
        if (closesocket(server) == SOCKET_ERROR) { //Ошибка закрытия сокета
            cout << "Close socket failed with error: " << WSAGetLastError() << endl;
            return -1;
        }
        WSACleanup();
    } */

    VisaDevice device("TCPIP0::K-N9020B-11111::inst0::INSTR");
    std::string data{};

    TcpServer server;
    server.read_data(data);

    device.query("*IDN?", data);

    printf("%s\n", data.c_str());

    return 0;
}