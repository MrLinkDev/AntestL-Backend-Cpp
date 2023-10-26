/**
 * \file
 * \brief Файл исходного кода, в котором реализованы методы для класа TesartRbd
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#include "tesart_rbd.hpp"
#include "../../utils/exceptions.hpp"
#include "../../utils/string_utils.hpp"

/**
 * \brief Запрос статуса оси ОПУ по номеру оси
 *
 * \param [in] axis_num Номер оси
 *
 * \return Полученный статус
 */
long long TesartRbd::status(int axis_num) {
    std::string str_answer{};

    std::stringstream stream{};
    long long answer{};

    axes[axis_num].clear();

    try {
        str_answer = axes[axis_num].send("TRJSTAT\r", true);
        str_answer = string_utils::lstrip(str_answer, 'H');
    } catch (std::invalid_argument inv_arg) {
        str_answer = axes[axis_num].send("TRJSTAT\r", true);
        str_answer = string_utils::lstrip(str_answer, 'H');
    }

    stream << std::hex << str_answer;
    stream >> answer;

    logger::log(LEVEL_TRACE, "Status = {} ({})", answer, str_answer);

    return answer;
}

/**
 * \brief Запрос статуса оси ОПУ по номеру оси по указателю на нужную ось
 *
 * \param [in] axis Указатель на ось
 *
 * \return Полученный статус
 */
long long TesartRbd::status(VisaDevice *axis) {
    std::string str_answer{};

    std::stringstream stream{};
    long long answer{};

    axis->clear();

    try {
        str_answer = axis->send("TRJSTAT\r", true);
        str_answer = string_utils::lstrip(str_answer, 'H');
    } catch (std::invalid_argument inv_arg) {
        str_answer = axis->send("TRJSTAT\r", true);
        str_answer = string_utils::lstrip(str_answer, 'H');
    }

    stream << std::hex << str_answer;
    stream >> answer;

    logger::log(LEVEL_TRACE, "Status = {} ({})", answer, str_answer);

    return answer;
}

/**
 * \brief Конструктор, в который передаётся адрес (или список одресов,
 * разделённых символом ';')
 *
 * \param [in] device_addresses адрес оси (список адресов осей)
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 * \endcode
 */
TesartRbd::TesartRbd(const std::string &device_addresses) {
    std::vector<std::string> address_list = string_utils::split(device_addresses, ADDRESS_DELIMITER);

    for (int i = 0; i < address_list.size(); ++i) {
        logger::log(LEVEL_TRACE, "Address [{}] = {}", i, address_list[i]);
    }

    for (const std::string &address : address_list) {
        axes.push_back(VisaDevice(address));

        axes[axes.size() - 1].connect();

        axes[axes.size() - 1].send("PROMPT 0\r");
        axes[axes.size() - 1].send("CLRFAULT\r");
        axes[axes.size() - 1].send("EN\r");

        axes[axes.size() - 1].clear();
        std::this_thread::sleep_for(TESART_RBD_SLEEP_TIME_INIT);

        if (!(status(&axes[axes.size() - 1]) & BIT_REF_SET)) {
            axes[axes.size() - 1].send("MH\r");
        }
    }

    init_params(axes.size());
}

/**
 * \brief Метод, возвращающий значение флага connected
 *
 * \warning Если был передан список адресов осей и с одной из осей не удалось
 * установить соединение, то будет возвращён false.
 *
 * \return Если все оси подключены - true. Если хотя бы одна из них не
 * подключена - false.
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 * if (!rbd->is_connected()) {
 *     exit(1);
 * }
 * \endcode
 */
bool TesartRbd::is_connected() {
    bool connected = true;

    for (const VisaDevice &axis : axes) {
        connected &= axis.is_connected();
    }

    return connected;
}

/**
 * \brief Метод позволяет проверить, остановлена ли ось
 *
 * \param [in] axis_num Номер оси, для которой осуществляется проверка
 *
 * \return Если ось остановлена - true. В противном случае - false.
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 * if (!rbd->is_stoppde(0)) {
 *     rbd->stop();
 * }
 * \endcode
 */
bool TesartRbd::is_stopped(int axis_num) {
    int status_code = status(axis_num);

    Logger::log(LEVEL_TRACE, "IN POS = {} ({})", bool(status_code & BIT_IN_POS), status_code & BIT_IN_POS);
    Logger::log(LEVEL_TRACE, "MOVE BLOCK = {} ({})", bool(status_code & BIT_MOVE_BLOCK), status_code & BIT_MOVE_BLOCK);

    return bool(status_code & BIT_IN_POS) and !bool(status_code & BIT_MOVE_BLOCK);
}

/**
 * \brief Поворачивает ось до тех пор, пока она не достигнет требуемого угла
 *
 * \param [in] pos Требуемый угол
 * \param [in] axis_num Номер оси
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 * rbd->move(12.7f, 0);
 * \endcode
 */
void TesartRbd::move(float pos, int axis_num) {
    logger::log(LEVEL_TRACE, "Axis {} pos = {}", axis_num, pos);

    axes[axis_num].send(
            "ORDER 0 {} {} 8192 {} {} 0 -1 0 0\r",
            int(pos * SCALE), velocity, acceleration, acceleration);
    axes[axis_num].send("MOVE 0\r");

    while(!is_stopped(axis_num)) {
        std::this_thread::sleep_for(100ms);
    }
}

/**
 * \brief Остановка всех осей
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 * if (!rbd->is_stoppde(0)) {
 *     rbd->stop();
 * }
 * \endcode
 */
void TesartRbd::stop() {
    for (int axis_num = 0; axis_num < axes.size(); ++axis_num) {
        axes[axis_num].send("STOP\r");

        while (!is_stopped(axis_num)) {
            std::this_thread::sleep_for(50ms);
        }
    }
}

/**
 * \brief Поворачивает ось ОПУ на требуемый угол
 *
 * \param [in] angle Требуемый угол
 * \param [in] axis_num Номер ОСИ
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 * rbd.set_angle(5.0f, 1);
 * \endcode
 */
void TesartRbd::set_angle(float angle, int axis_num) {
    this->start_angle[axis_num] = angle;
    this->stop_angle[axis_num] = angle;

    this->current_angle[axis_num] = angle;

    this->points[axis_num] = 1;
    this->current_point[axis_num] = 0;

    logger::log(LEVEL_TRACE, "TesartRbd: move({}, {})", angle, axis_num);
    move(angle, axis_num);
}

/**
 * \brief Задаёт диапазон изменения угла для определённой оси
 *
 * \param [in] start_angle Начальное значение угла
 * \param [in] stop_angle Конечное значение угла
 * \param [in] points Количество точек
 * \param [in] axis_num Номер оси
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 * rbd.set_angle_range(-30, 30, 11, 0);
 * \endcode
 */
void TesartRbd::set_angle_range(float start_angle, float stop_angle, int points, int axis_num) {
    this->start_angle[axis_num] = start_angle;
    this->stop_angle[axis_num] = stop_angle;

    this->points[axis_num] = points;

    angle_step[axis_num] =
            this->points[axis_num] <= 1 ?
            0 : (this->stop_angle[axis_num] - this->start_angle[axis_num]) / (this->points[axis_num] - 1);

    this->current_angle[axis_num] = start_angle;
    this->current_point[axis_num] = 0;
}

/**
 * \brief Переход к следующей угловой точке
 *
 * \param [in] axis_num Номер оси ОПУ
 *
 * \return Если угловая точка находится в пределах диапазона изменения угла,
 * то возвращается ANGLE_MOVE_OK. Если угловая точка находится на границе
 * диапазона, то возвращает ANGLE_MOVE_BOUND.
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 *
 * rbd.set_angle_range(-30, 30, 11, 0);
 * while (rbd->next_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "ОПУ повёрнута на следующую угловую точку" << std::endl;
 * }
 * \endcode
 */
int TesartRbd::next_angle(int axis_num) {
    if (current_point[axis_num] == points[axis_num] - 1) {
        return ANGLE_MOVE_BOUND;
    }

    current_angle[axis_num] += angle_step[axis_num];
    ++current_point[axis_num];

    move(current_angle[axis_num], axis_num);

    return ANGLE_MOVE_OK;
}

/**
 * \brief Переход к предыдущей угловой точке
 *
 * \param [in] axis_num Номер оси ОПУ
 *
 * \return Если угловая точка находится в пределах диапазона изменения угла,
 * то возвращается ANGLE_MOVE_OK. Если угловая точка находится на границе
 * диапазона, то возвращает ANGLE_MOVE_BOUND.
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 *
 * rbd.set_angle_range(-30, 30, 11, 0);
 * while (rbd->prev_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "ОПУ повёрнута на предыдущую угловую точку" << std::endl;
 * }
 * \endcode
 */
int TesartRbd::prev_angle(int axis_num) {
    if (current_point[axis_num] == 0) {
        return ANGLE_MOVE_BOUND;
    }

    current_angle[axis_num] -= angle_step[axis_num];
    --current_point[axis_num];

    move(current_angle[axis_num], axis_num);

    return ANGLE_MOVE_OK;
}

/**
 * \brief Устанавливает угловую точку, соответствующую началу
 * диапазона.
 *
 * \param [in] axis_num Номер оси ОПУ
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 *
 * rbd.set_angle_range(-30, 30, 11, 0);
 * while (rbd->next_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "ОПУ повёрнута на следующую угловую точку" << std::endl;
 * }
 *
 * rbd->move_to_start_angle(0);
 * \endcode
 */
void TesartRbd::move_to_start_angle(int axis_num) {
    current_angle[axis_num] = start_angle[axis_num];
    current_point[axis_num] = 0;

    move(current_angle[axis_num], axis_num);
}

/**
 * \brief Устанавливает угловую точку, соответствующую концу
 * диапазона.
 *
 * \param [in] axis_num Номер оси ОПУ
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 *
 * rbd.set_angle_range(-30, 30, 11, 0);
 * while (rbd->prev_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "ОПУ повёрнута на предыдущую угловую точку" << std::endl;
 * }
 *
 * rbd->move_to_stop_angle(0);
 * \endcode
 */
void TesartRbd::move_to_stop_angle(int axis_num) {
    current_angle[axis_num] = stop_angle[axis_num];
    current_point[axis_num] = points[axis_num] - 1;

    move(current_angle[axis_num], axis_num);
}

/**
 * \brief Получает позицию требуемой оси
 *
 * \param [in] axis_num Ось, позицию которой требется узнать
 *
 * \return Позиция, в которой находится ось
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 *
 * rbd.set_angle_range(-30, 30, 11, 0);
 * while (rbd->prev_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "Pos = " << rbd->get_pos(0) << std::endl;
 * }
 *
 * rbd->move_to_stop_angle(0);
 * \endcode
 */
float TesartRbd::get_pos(int axis_num) {
    std::string str_answer{};
    float answer{};

    axes[axis_num].clear();

    str_answer = axes[axis_num].send("PFB\r", true);
    answer = stof(str_answer) / SCALE;

    return answer;
}

/**
 * \brief Получает количество осей
 *
 * \return Количество осей
 *
 * **Пример**
 * \code
 * RbdDevice *rbd = new TesartRbd("TCPIP0::localhost::5025::SOCKET;TCPIP0::localhost::5026::SOCKET");
 *
 * for (int i = 0; i < rbd->get_axis_count(); ++i) {
 *     rbd.set_angle_range(-30, 30, 11, i);
 *
 *     while (rbd->prev_angle(i) != ANGLE_MOVE_BOUND) {
 *         std::cout << "Pos = " << rbd->get_pos(i) << std::endl;
 *     }
 *
 *     rbd->move_to_stop_angle(i);
 * }
 * \endcode
 */
int TesartRbd::get_axes_count() {
    return axes.size();
}
