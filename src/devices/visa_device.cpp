/**
 * \file
 * \brief Файл исходного кода, в котором реализованы конструкторы и
 * методы для класа VisaDevice
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#include "visa_device.hpp"
#include "../utils/exceptions.hpp"


/**
 * \brief Метод, позволяющий отправить команду на устройство
 *
 * Перед отправкой, к команде добавляется символ окончания посылки,
 * затем полученная строка передаётся на устройство.
 *
 * \param [in] command Команда, отправляемая на устройство
 * \return Если команда была отправлена успешно, то возвращается SUCCESS.
 * В противном случае - FAILURE.
 */
int VisaDevice::write(std::string command) {
    logger::log(LEVEL_TRACE, "WRITE: {}", command);
    command += device_config.termination;

    auto command_buffer = reinterpret_cast<ViConstBuf>(command.c_str());
    status = viWrite(device, command_buffer, command.length(), ret_count);

    if (status < VI_SUCCESS) {
        return FAILURE;
    }

    return SUCCESS;
}

/**
 * \brief Метод, позволяющий считать данные с устройства
 *
 * Чтение осуществляется до тех пор, пока не встретится символ, которым
 * заканчивается посылка
 *
 * \return Возвращает считанные данные без символа конца посылки
 */
std::string VisaDevice::read() {
    std::string data{};
    ViChar buffer[BUFFER_SIZE];

    while (true) {
        status = viRead(device, reinterpret_cast<ViPBuf>(buffer), BUFFER_SIZE, ret_count);

        if (status < VI_SUCCESS) {
            return std::string{};
        }

        for (char b : buffer) {
            if (b == device_config.termination) {
                logger::log(LEVEL_TRACE, "READ: {}", data);
                return data;
            }

            data.push_back(b);
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

/**
 * \brief Метод, совмещающий в себе методы write() и read().
 *
 * Данный метод отправляет команду, с помощью метода write(),
 * а затем считывает данные с прибора, с помощью метода read().
 *
 * \param [in] command Отправляемая команда
 *
 * \return Возвращает считанные данные
 */
std::string VisaDevice::query(std::string command) {
    std::string data{};

    if (write(std::move(command)) == FAILURE) {
        return std::string{};
    }

    data = read();
    return data;
}

/**
 * \brief Конструктор, который создаёт объект устройства с адресом,
 * переданным в качестве аргумента
 *
 * \param [in] device_address Адрес устройства
 *
 * **Пример**
 * \code
 * VisaDevice device("TCPIP0::localhost::5025::SOCKET");
 * \endcode
 */
VisaDevice::VisaDevice(std::string device_address) {
    device_config.address = std::move(device_address);
}

/**
 * \brief Деструктор, который выполняет отключение от прибора
 */
VisaDevice::~VisaDevice() {
    if (connected) {
        clear();
        viClose(device);

        connected = false;
    }
}

/**
 * \brief Метод, осуществляющий подключение к прибору.
 *
 * Инициализируется менеджер ресурсов, с помощью которого осуществляется
 * подключение к прибору. Если подключение было установлено, то флаг
 * connected становится true. В противном случае - false.
 *
 * **Пример**
 * \code
 * VisaDevice device("TCPIP0::localhost::5025::SOCKET");
 * device.connect();
 *
 * if (device.is_connected()) {
 *     std::cout << "Устройство подключено" << std::endl;
 * }
 * \endcode
 */
void VisaDevice::connect() {
    status = viOpenDefaultRM(&resource_manager);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't open resource manager");

        connected = false;
        return;
    }

    logger::log(LEVEL_TRACE, "Connecting to device with address {}", device_config.address);

    status = viOpen(
            resource_manager,
            device_config.address.c_str(),
            VI_NULL,
            VI_NULL,
            &device);

    if (status < VI_SUCCESS) {
        connected = false;
        return;
    } else {
        logger::log(LEVEL_DEBUG, "Connected to device with address {}", device_config.address);
    }

    status = viSetAttribute(device, VI_ATTR_TMO_VALUE, DEFAULT_TIMEOUT);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't set timeout");

        connected = false;
        return;
    }

    status = viSetAttribute(device, VI_ATTR_TERMCHAR, DEFAULT_VISA_TERM);

    if (status < VI_SUCCESS) {
        logger::log(LEVEL_ERROR, "Can't set termination character");

        connected = false;
        return;
    }

    connected = true;
}

/**
 * \brief Метод, возвращающий значение флага connected
 *
 * \return Значение флага connected
 */
bool VisaDevice::is_connected() const {
    return connected;
}

/**
 * \brief Очищает входящий и исходящие буферы устройства
 */
void VisaDevice::clear() const {
    viClear(device);
}

/**
 * \brief Отправляет команду "*IDN?" на подключенное устройство и ожидает ответа
 *
 * \return Информацию о подключенном приборе
 */
std::string VisaDevice::idn() {
    std::string device_info;

    device_info = query(CMD_IDN);

    if (device_info.empty()) {
        logger::log(LEVEL_WARN, "No IDN data from device");
    } else {
        logger::log(LEVEL_DEBUG, "Device info: {}", device_info);
    }

    return device_info;
}

/**
 * \brief Отправляет команду "*OPC?" на подключенное устройство, ожидает ответ и,
 * в зависимости от полученного ответа, возвращает OPC_PASS или OPC_WAIT.
 *
 * \return Если прибор выполняет действия, то возвращает OPC_WAIT.
 * Если прибор завершил выполнение действий, то возвращает OPC_PASS.
 * Если возникли ошибки, то возвращает FAILURE.
 */
int VisaDevice::opc() {
    std::string opc_info{};

    opc_info = query(CMD_OPC);

    if (opc_info.empty()) {
        logger::log(LEVEL_WARN, "No OPC data from device");
    } else {
        if (opc_info == OPC_PASS_STR_KEYSIGHT || opc_info == OPC_PASS_STR_PLANAR) {
            return OPC_PASS;
        } else {
            return OPC_WAIT;
        }
    }

    return FAILURE;
}

/**
 * \brief Отправляет команду "SYSTEM:ERROR?" на подключенное устройство и считывает
 * сведения об ошибках, возникших на устройстве.
 *
 * \return Если на устройстве не возникло ошибок, то вернёт код NO_ERRORS. Если на
 * устройстве возникли ошибки, то возвращает ERRORS. Если, в процессе запроса, возникли
 * проблемы, то вернёт FAILURE.
 */
int VisaDevice::err() {
    std::string error_info = query(CMD_ERR);

    if (error_info.empty()) {
        logger::log(LEVEL_WARN, "No error data from device");
    } else {
        if (error_info == NO_ERROR_STR) {
            return NO_ERRORS;
        } else {
            logger::log(LEVEL_ERROR, error_info);
            return ERRORS;
        }
    }

    return FAILURE;
}

/**
 * \brief Ожидание завершения выполнения команды.
 *
 * Цикл внутри метода будет выполняться до тех пор, пока от функции opc()
 * не вернётся ответ OPC_PASS.
 */
void VisaDevice::wait() {
    int opc_status;

    do {
        opc_status = opc();

        if (opc_status == FAILURE) {
            logger::log(LEVEL_ERROR, OPC_ERROR_MSG);
            throw antestl_exception(OPC_ERROR_MSG, OPC_ERROR_CODE);
        }

    } while (opc_status != OPC_PASS);
}

/**
 * \brief Отправка данных на прибор и чтение ответа от прибора.
 *
 * Данный метод позволяет отправить требуемую команду. Если команда оканчивается
 * символом '?' или передан аргумент read_data = true, то ожидается ответ от прибора.
 *
 * \param [in] command Отправляемая команда
 * \param [in] read_data Флаг, показывающий, требуется ли ожидать данные от устройства
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
 *     vna.send(":SENSe:SWEep:POINts 201");
 *     std::string answer = vna.send(":SENSe:SWEep:POINts?");
 *
 *     std::cout << answer << std::endl;        // Будет выведено число 201
 * }
 * \endcode
 */
std::string VisaDevice::send(std::string command, bool read_data) {
    std::string data{};

    if (command[command.length() - 1] == '?') {
        data = query(command);

        if (data.empty()) {
            logger::log(LEVEL_ERROR, READ_ERROR_MSG);
            throw antestl_exception(READ_ERROR_MSG, READ_ERROR_CODE);
        }
    } else {
        if (write(command) == FAILURE) {
            logger::log(LEVEL_ERROR, WRITE_ERROR_MSG);
            throw antestl_exception(WRITE_ERROR_MSG, WRITE_ERROR_CODE);
        }

        if (read_data) {
            data = read();

            if (data.empty()) {
                logger::log(LEVEL_ERROR, READ_ERROR_MSG);
                throw antestl_exception(READ_ERROR_MSG, READ_ERROR_CODE);
            }
        }
    }

    return data;
}

/**
 * \brief Отправка данных на прибор и чтение ответа от прибора. Также,
 * ожидается завершение выполнения действий прибором.
 *
 * Метод ожидает завершение выполнения команды. Если строка заканчивается
 * символом '?' или передан флаг read_data = true, то метод ожидает ответ
 * от прибора. В противном случае возвращается пустая строка.
 *
 * \param [in] command Отправляемая команда
 * \param [in] read_data Флаг, показывающий, требуется ли ожидать данные от устройства
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
 *     vna.send(":SENSe:SWEep:POINts 201");
 *     vna.send_wait("INIT");
 *
 *     std::string data = vna.send(":CALCULATE:MEASURE:DATA:SDATA?");
 * }
 * \endcode
 */
std::string VisaDevice::send_wait(std::string command) {
    std::string data{};

    data = send(std::move(command));
    wait();

    return data;
}

/**
 * \brief Отправка данных на прибор и чтение ответа от прибора.
 * Также, проверяется наличие ошибок на приборе.
 *
 * Метод проверяет наличие ошибок на приборе после выполнения
 * команды. Если строка заканчивается символом '?' или передан
 * флаг read_data = true, то функция ожидает ответ от прибора.
 *
 * \param [in] command Отправляемая команда
 * \param [in] read_data Флаг, показывающий, требуется ли ожидать данные от устройства
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
std::string VisaDevice::send_err(std::string command) {
    std::string data{};

    data = send(std::move(command));

    if (err() == ERRORS) {
        logger::log(LEVEL_ERROR, DEVICE_ERROR_MSG);
        throw antestl_exception(DEVICE_ERROR_MSG, DEVICE_ERROR_CODE);
    }

    return data;
}

/**
 * \brief Отправка данных на прибор и чтение ответа от прибора. Также,
 * ожидается завершение выполнения действий прибором и проверяется наличие
 * ошибок на приборе.
 *
 * Метод ожидает завершение выполнения команды проверяет наличие ошибок
 * на приборе после выполнения команды. Если строка заканчивается
 * символом '?' или был передан флаг read_data = true, то метод ожидает
 * ответ от прибора. В противном случае возвращается пустая строка.
 *
 * \param [in] command Отправляемая команда
 * \param [in] read_data Флаг, показывающий, требуется ли ожидать данные от устройства
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
std::string VisaDevice::send_wait_err(std::string command) {
    std::string data{};

    send_wait(std::move(command));

    if (err() == ERRORS) {
        logger::log(LEVEL_ERROR, DEVICE_ERROR_MSG);
        throw antestl_exception(DEVICE_ERROR_MSG, DEVICE_ERROR_CODE);
    }

    return data;
}
