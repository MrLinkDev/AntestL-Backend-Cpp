/**
 * \file
 * \brief Файл исходного кода, в котором реализованы методы для класа DeviceSet
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#include "device_set.hpp"
#include "rbd/demo_rdb.hpp"

/**
 * \brief Метод, позволяющий осуществить подключение к устройству.
 *
 * \param [in] device_type Тип устройства
 * \param [in] device_model Модель устройства
 * \param [in] device_address Адрес устройства
 *
 * \return Если устройство было подключено, то возвращает true. В противном случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 * if (device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET")) {
 *     std::cout << "Успешно" std::endl;
 * } else {
 *     std::cout << "Ошибка" << std::endl;
 *     exit(1);
 * }
 * \endcode
 */
bool DeviceSet::connect(int device_type, std::string device_model, const std::string &device_address) {
    std::transform(device_model.begin(), device_model.end(), device_model.begin(), ::toupper);

    try {
        switch (device_type) {
            case DEVICE_VNA:
                logger::log(LEVEL_TRACE, "Connecting to VNA");

                if (device_model == "M9807A") {
                    vna = new KeysightM9807A(device_address);
                    vna->preset();
                } else {
                    return false;
                }

                return vna->is_connected();
            case DEVICE_GEN:
                logger::log(LEVEL_TRACE, "Connecting to external gen");
                ext_gen = new KeysightGen(device_address);

                return ext_gen->is_connected();
            case DEVICE_RBD:
                logger::log(LEVEL_TRACE, "Connecting to RBD");

                if (device_model == "TESART_RBD") {
                    rbd = new TesartRbd(device_address);
                } else if (device_model == "UPKB_RBD") {
                    //TODO: Добавить инициализацию объекта
                } else if (device_model == "DEMO_RBD") {
                    rbd = new DemoRbd(device_address);
                } else {
                    return false;
                }

                return rbd->is_connected();
            default:
                return false;
        }
    } catch (const antestl_exception &exception) {
        return false;
    }
}

/**
 * \brief Отключает все устройства и уничтожает созданные объекты
 */
void DeviceSet::disconnect() {
    delete vna;
    delete ext_gen;
    delete rbd;
}

/**
 * \brief Настраивает ВАЦ для требуемого измерения
 *
 * \param [in] meas_type Тип измерения
 * \param [in] rbw Полоса разрешающего фильтра
 * \param [in] source_port Зондирующий порт
 * \param [in] using_ext_gen Флаг, показывающий, используется ли внешний генератор
 *
 * \return Если ВАЦ был сконфигурирован, то возвращает true. В противном случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.configure(MEAS_TRANSITION, 1e3, 1, false);
 * \endcode
 */
bool DeviceSet::configure(int meas_type, float rbw, int source_port, bool using_ext_gen) {
    try {
        vna->full_preset();
        logger::log(LEVEL_TRACE, "Made full preset");

        vna->init_channel();
        logger::log(LEVEL_TRACE, "Default channel initialized");

        vna->configure(meas_type, rbw, source_port);

        this->meas_type = meas_type;
        this->using_ext_gen = using_ext_gen;
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't configure VNA");
        return false;
    }

    traces_configured = false;

    logger::log(LEVEL_DEBUG, "VNA configured");
    return true;
}

/**
 * \brief Устанавливает требуемую мощность зондирующего порта
 *
 * \param [in] power Требуемое значение мощности
 *
 * \return Если требуемое значение мощности было установлено, то
 * возвращает true. В противном случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_power(-20.0f);
 * \endcode
 */
bool DeviceSet::set_power(float power) {
    try {
        if (using_ext_gen) {
            ext_gen->set_power(power);
            logger::log(LEVEL_TRACE, "External gen power = {}", power);
        } else {
            vna->set_power(power);
            logger::log(LEVEL_DEBUG, "VNA power = {}", power);
        }
    } catch (const antestl_exception &exception) {
        if (using_ext_gen) {
            logger::log(LEVEL_ERROR, "Can't change power on external generator");
        } else {
            logger::log(LEVEL_ERROR, "Can't change power on VNA");
        }

        return false;
    }

    return true;
}

/**
 * \brief Устанавливает требуемое значение частоты зондирующего сигнала
 *
 * \param [in] freq Требуемая частота зондирующего сигнала
 *
 * \return Если требуемое значение частоты было установлено, то
 * возвращает true. В противном случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_freq(1e9);
 * \endcode
 */
bool DeviceSet::set_freq(double freq) {
    try {
        if (using_ext_gen) {
            ext_gen->set_freq(freq);
            logger::log(LEVEL_DEBUG, "External gen frequency = {}", freq);
        } else {
            vna->set_freq(freq);
            logger::log(LEVEL_DEBUG, "VNA frequency = {}", freq);
        }
    } catch (const antestl_exception &exception) {
        if (using_ext_gen) {
            logger::log(LEVEL_ERROR, "Can't change frequency on external generator");
        } else {
            logger::log(LEVEL_ERROR, "Can't change frequency on VNA");
        }

        return false;
    }

    return true;
}

/**
 * \brief Устанавливает диапазон изменения частоты зондирующего сигнала
 *
 * \param [in] start_freq Начальное значение частоты
 * \param [in] stop_freq Конечное значение частоты
 * \param [in] points Количество частотных точек
 *
 * \return Если диапазон был установлен успешно, то возвращает true.
 * В противном случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_freq_range(1e9, 2e9, 11);
 * \endcode
 */
bool DeviceSet::set_freq_range(double start_freq, double stop_freq, int points) {
    try {
        if (using_ext_gen) {
            ext_gen->set_freq_range(start_freq, stop_freq, points);
            logger::log(LEVEL_DEBUG, "External gen frequency range = [{}; {}] ({} points)", start_freq, stop_freq, points);
        } else {
            vna->set_freq_range(start_freq, stop_freq, points);
            logger::log(LEVEL_DEBUG, "VNA frequency range = [{}; {}] ({} points)", start_freq, stop_freq, points);
        }
    } catch (const antestl_exception &exception) {
        if (using_ext_gen) {
            logger::log(LEVEL_ERROR, "Can't change frequency range on external generator");
        } else {
            logger::log(LEVEL_ERROR, "Can't change frequency range on VNA");
        }

        return false;
    }

    return true;
}

/**
 * \brief Переход к следующей частотной точке
 *
 * \return Если следующая частотная точка находится в пределах
 * границ изменения, то возвращает FREQ_MOVE_OK. Если точка
 * находится на границе, то возвращает FREQ_MOVE_BOUND.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_freq_range(1e9, 2e9, 11);
 *
 * while (device_set.next_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана следующая частотная точка" << std::endl;
 * }
 * \endcode
 */
int DeviceSet::next_freq() {
    try {
        return ext_gen->next_freq();
    } catch (const antestl_exception &exception) {
        return -1;
    }
}

/**
 * \brief Переход к предыдущей частотной точке
 *
 * \return Если предыдущая частотная точка находится в пределах
 * границ изменения, то возвращает FREQ_MOVE_OK. Если точка
 * находится на границе, то возвращает FREQ_MOVE_BOUND.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_freq_range(1e9, 2e9, 11);
 *
 * while (device_set.prev_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана предыдущая частотная точка" << std::endl;
 * }
 * \endcode
 */
int DeviceSet::prev_freq() {
    try {
        return ext_gen->prev_freq();
    } catch (const antestl_exception &exception) {
        return -1;
    }
}

/**
 * \brief Устанавливает частотную точку, соответствующую началу
 * диапазона.
 *
 * \return Если действие прошло успешно, возвращает true. В противном
 * случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_freq_range(1e9, 2e9, 11);
 *
 * while (device_set.next_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана следующая частотная точка" << std::endl;
 * }
 *
 * if (device_set.move_to_start_freq()) {
 *     std::cout << "Выбрана частотная точка в начале диапазона" << std::endl;
 * }
 * \endcode
 */
bool DeviceSet::move_to_start_freq() {
    if (using_ext_gen) {
        try {
            ext_gen->move_to_start_freq();
        } catch (const antestl_exception &exception) {
            logger::log(LEVEL_ERROR, "Can't set frequency on external gen");
            return false;
        }
    }

    return true;
}

/**
 * \brief Получает текущее значение частоты зондирующего сигнала на подключенном генераторе
 *
 * \return Значение частоты зондирующего сигнала генератора
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.connect(DEVICE_GEN, "keysight_gen", "TCPIP0::localhost::5026::SOCKET");
 * device_set.set_freq_range(1e9, 2e9, 11);
 *
 * while (device_set.next_freq() != FREQ_MOVE_BOUND) {
 *     std::cout << "Выбрана следующая частотная точка. f = " << device_set.get_current_freq()) << std::endl;
 * }
 * \endcode
 */
double DeviceSet::get_current_freq() {
    if (ext_gen != nullptr && ext_gen->is_connected()) {
        try {
            return ext_gen->get_current_freq();
        } catch (const antestl_exception &exception) {
            logger::log(LEVEL_ERROR, "Can't get current frequency from gen");
            return 0.0;
        }
    } else {
        return 0.0;
    }
}

/**
 * \brief Получает вектор частотных точек
 *
 * \return Вектор частотных точек
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_freq_range(1e9, 2e9, 11);
 *
 * std::vector<double> freq_list = device_set.get_freq_list();
 *
 * for (double item: freq_list) {
 *     std::cout << "f = " << item << std::endl;
 * }
 * \endcode
 */
std::vector<double> DeviceSet::get_freq_list() {
    std::vector<double> freq_list{};

    for (int pos = 0; pos < vna->get_points(); ++pos) {
        freq_list.push_back(vna->get_start_freq() + pos * vna->get_freq_step());
    }

    return freq_list;
}

/**
 * \brief Устанавливает требуемое значение угла у определённой оси ОПУ
 *
 * \param [in] angle Требуемое значение угла
 * \param [in] axis_num Номер оси ОПУ
 *
 * \return Если установка угла прошла успешно, возвращает true. В противном
 * случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_RBD, "demo_rbd", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_angle(-30.0f, 0);
 * \endcode
 */
bool DeviceSet::set_angle(float angle, int axis_num) {
    try {
        rbd->set_angle(angle, axis_num);
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't set angle on RBD");
        return false;
    }

    logger::log(LEVEL_DEBUG, "RBD (axis {}): angle = {}", axis_num, angle);
    return true;
}

/**
 * \brief Устанавливает диапазон изменения угла на определённой оси ОПУ
 *
 * \param [in] start_angle Начальное значение диапазона
 * \param [in] stop_angle Конечное значение диапазона
 * \param [in] points Количество точек
 * \param [in] axis_num Номер оси ОПУ
 *
 * \return Если установка диапазона прошла успешно, то возвращает true.
 * В противном случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_RBD, "demo_rbd", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_angle_range(-30.0f, 30.0f, 11, 0);
 * \endcode
 */
bool DeviceSet::set_angle_range(float start_angle, float stop_angle, int points, int axis_num) {
    try {
        rbd->set_angle_range(start_angle, stop_angle, points, axis_num);
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't set angle range on RBD");
        return false;
    }

    logger::log(LEVEL_DEBUG, "RBD (axis {}): angle range = [{}; {}] ({} points)", axis_num, start_angle, stop_angle, points);
    return true;
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
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_RBD, "demo_rbd", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_angle_range(-30.0f, 30.0f, 11, 0);
 *
 * while (device_set.next_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "Выбрана следующая угловая точка" << std::endl;
 * }
 * \endcode
 */
int DeviceSet::next_angle(int axis_num) {
    try {
        return rbd->next_angle(axis_num);
    } catch (const antestl_exception &exception) {
        return -1;
    }
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
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_RBD, "demo_rbd", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_angle_range(-30.0f, 30.0f, 11, 0);
 *
 * while (device_set.prev_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "Выбрана предыдущая угловая точка" << std::endl;
 * }
 * \endcode
 */
int DeviceSet::prev_angle(int axis_num) {
    try {
        return rbd->prev_angle(axis_num);
    } catch (const antestl_exception &exception) {
        return -1;
    }
}

/**
 * \brief Устанавливает угловую точку, соответствующую началу
 * диапазона.
 *
 * \param [in] axis_num Номер оси ОПУ
 *
 * \return Если действие прошло успешно, возвращает true. В противном
 * случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_RBD, "demo_rbd", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_angle_range(-30.0f, 30.0f, 11, 0);
 *
 * while (device_set.next_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "Выбрана следующая угловая точка" << std::endl;
 * }
 *
 * if (device_set.move_to_start_angle(0)) {
 *     std::cout << "Выбрана угловая точка в начале диапазона" << endl;
 * }
 * \endcode
 */
bool DeviceSet::move_to_start_angle(int axis_num) {
    try {
        rbd->move_to_start_angle(axis_num);
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't set angle on RBD (axis {})", axis_num);
        return false;
    }

    return true;
}

/**
 * \brief Получает список углов, на которые развёрнуты оси ОПУ
 *
 * \return Строка, содержащая значения углов
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_RBD, "demo_rbd", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_angle_range(-30.0f, 30.0f, 11, 0);
 *
 * while (device_set.next_angle(0) != ANGLE_MOVE_BOUND) {
 *     std::cout << "angle = " << device_set.get_current_angles() << std::endl;
 * }
 * \endcode
 */
std::string DeviceSet::get_current_angles() {
    std::string angle_list{};

    if (rbd != nullptr && rbd->is_connected()) {
        for (int axis = 0; axis < rbd->get_axes_count(); ++axis) {
            if (axis == 0) {
                angle_list = std::to_string(rbd->get_pos(axis));
            } else {
                angle_list += "," + std::to_string(rbd->get_pos(axis));
            }
        }
    }

    return angle_list;
}

/**
 * \brief Переводит переключатели в заданные положения
 *
 * \param [in] path_list Вектор положений переключателей
 *
 * \return Если действие выполнено успешно, возвращает true. В противном
 * случае - false.
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 * device_set.set_path({1, 4, 2, 5});
 * \endcode
 */
bool DeviceSet::set_path(std::vector<int> path_list) {
    try {
        vna->set_path(std::move(path_list));
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't change switch paths on VNA");
        return false;
    }

    logger::log(LEVEL_DEBUG, "Changed switch paths on VNA");
    return true;
}

/**
 * \brief Метод позволяет получить число переключателей, подключенных
 * к ВАЦ.
 *
 * \return Количество модулей
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 *
 * if (device_set.get_vna_switch_module_count() <= 0) {
 *     std::cout << "Переключатели не подключены к ВАЦ" << std::endl;
 * }
 * \endcode
 */
int DeviceSet::get_vna_switch_module_count()  {
    if (vna != nullptr && vna->is_connected()) {
        return vna->get_switch_module_count();
    } else {
        return 0;
    }
}

/**
 * \brief Метод позволяет проверить, используется ли внешний генератор
 *
 * \return Флаг, который показывает, используется ли внешний генератор
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 *
 * if (device_set.is_using_ext_gen()) {
 *     std::cout << "f = " << device_set.get_current_freq() std::endl;
 * }
 * \endcode
 */
bool DeviceSet::is_using_ext_gen() const {
    return using_ext_gen;
}

/**
 * \brief Метод, осуществляющий проведение измерения и получение результатов
 * измерения
 *
 * \warning Перед использованием данного метода требуется осуществить подключение
 * к нужным устройствам и произвести их настройку!
 *
 * \param [in] port_list Список портов, для которых требуется провести измерение
 *
 * \return Полученные результаты измерений
 *
 * **Пример**
 * \code
 * DeviceSet device_set();
 *
 * device_set.connect(DEVICE_VNA, "m9807a", "TCPIP0::localhost::5025::SOCKET");
 *
 * device_set.configure(MEAS_TRANSITION, 1e3, 1, false);
 * device_set.set_power(0.0);
 * device_set.set_freq_range(1.2e9, 2.4e9, 101);
 *
 * data_t acquired_data = device_set.get_data({2, 4, 5});   // Сбор данных для второго, четвёртого и пятого портов
 * \endcode
 */
data_t DeviceSet::get_data(std::vector<int> port_list) {
    if (stop_requested) {
        logger::log(LEVEL_WARN, "Device set stops measuring");
        stop_requested = false;

        return data_t{};
    }

    logger::log(LEVEL_TRACE, "Preparing to acquire data");
    data_t acquired_data{};

    try {
        if (!traces_configured) {
            vna->create_traces(port_list, using_ext_gen);
            traces_configured = true;
        }

        logger::log(LEVEL_TRACE, "Traces created");
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't create traces");
        return acquired_data;
    }

    for (int port_pos = 0; port_pos < port_list.size(); ++port_pos) {
        if (stop_requested) {
            logger::log(LEVEL_WARN, "Device set stops measuring");
            stop_requested = false;

            return data_t{};
        }

        int port_num = port_list[port_pos];
        logger::log(LEVEL_TRACE, "Port = {}", port_num);

        if (meas_type == MEAS_TRANSITION && port_pos == 0) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_on();
                } else {
                    vna->rf_on(vna->get_source_port());
                }

                logger::log(LEVEL_TRACE, "Source port enabled");
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't enable source port");
                return acquired_data;
            }
        } else if (meas_type == MEAS_REFLECTION) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_on();
                } else {
                    vna->rf_on(port_num);
                }

                logger::log(LEVEL_TRACE, "Port {} enabled", port_num);
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't enable port {}", port_num);
                return acquired_data;
            }
        }

        if ((port_pos == 0 && meas_type == MEAS_TRANSITION) || meas_type == MEAS_REFLECTION) {
            try {
                vna->trigger();
                vna->init();

                logger::log(LEVEL_TRACE, "Measurements restarted");
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't restart measurement");
                return acquired_data;
            }
        }

        try {
            acquired_data.insert_iq_port_data(vna->get_data(port_pos));
            logger::log(LEVEL_TRACE, "Data for port {} acquired", port_num);
        } catch (int error_code) {
            logger::log(LEVEL_ERROR, "Can't acquire data for port {} from VNA", port_num);
            return acquired_data;
        }

        if (meas_type == MEAS_TRANSITION && port_pos == port_list.size() - 1) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_off();
                } else {
                    vna->rf_off(vna->get_source_port());
                }

                logger::log(LEVEL_TRACE, "Source port disabled");
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't disable source port");
                return acquired_data;
            }
        } else if (meas_type == MEAS_REFLECTION) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_off();
                } else {
                    vna->rf_off(port_num);
                }

                logger::log(LEVEL_TRACE, "Port {} disabled", port_num);
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't disable port {}", port_num);
                return acquired_data;
            }
        }
    }

    acquired_data.insert_angles(get_current_angles());

    if (using_ext_gen) {
        acquired_data.insert_freq(get_current_freq());
    } else {
        acquired_data.insert_freq_list(get_freq_list());
    }

    logger::log(LEVEL_DEBUG, "Data acquired");

    return acquired_data;
}

/**
 * \brief Присваивает флагу stop_request значение true, тем самым, останавливая
 * процес измерения
 */
void DeviceSet::request_stop() {
    stop_requested = true;
}

/**
 * \brief Сбрасывает флаг stop_request (присваивает значение false)
 */
void DeviceSet::reset_stop_request() {
    stop_requested = false;
}
