/**
 * \file
 * \brief Заголовочный файл, в котором реализован класс RbdDevice
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_RBD_DEVICE_HPP
#define ANTESTL_BACKEND_RBD_DEVICE_HPP

#include "../visa_device.hpp"
#include <vector>

/// Статус вращения ОПУ: находится в пределах допустимых границ
#define ANGLE_MOVE_OK       0xF2
/// Статус вращения ОПУ: находится на границе допустимых значений
#define ANGLE_MOVE_BOUND    0xF3

/// Разделитель адресов осей
#define ADDRESS_DELIMITER   ';'


/**
 * \brief Класс, в котором представлены методы для работы с ОПУ
 */
class RbdDevice {

protected:
    /// Вектор начальных углов
    std::vector<float> start_angle{};
    /// Вектор конечных углов
    std::vector<float> stop_angle{};

    /// Вектор угловых шагов
    std::vector<float> angle_step{};
    /// Вектор текущих позиций
    std::vector<float> current_angle{};

    /// Вектор количества точек
    std::vector<int> points{};
    /// Вектор текущих точек
    std::vector<int> current_point{};

    /**
     * \brief Метод инициализирующий параметры для требуемого количества осей
     *
     * \param axis_count Количество осей
     */
    void init_params(int axis_count = 0) {
        for (int i = 0; i < axis_count; ++i) {
            this->start_angle.push_back(0);
            this->stop_angle.push_back(0);

            this->angle_step.push_back(0);
            this->current_angle.push_back(0);

            this->points.push_back(0);
            this->current_point.push_back(0);
        }
    };

public:
    RbdDevice() = default;

    /**
     * \brief Метод, возвращающий значение флага connected
     *
     * \warning Если был передан список адресов осей и с одной из осей не удалось
     * установить соединение, то будет возвращён false.
     *
     * \return Если все оси подключены - true. Если хотя бы одна из них не
     * подключена - false.
     */
    virtual bool is_connected() {return false;};

    /**
     * \brief Поворачивает ось до тех пор, пока она не достигнет требуемого угла
     *
     * \param [in] pos Требуемый угол
     * \param [in] axis_num Номер оси
     */
    virtual void move(float pos, int axis_num) {};

    /**
     * \brief Остановка всех осей
     */
    virtual void stop() {};

    /**
     * \brief Поворачивает ось ОПУ на требуемый угол
     *
     * \param [in] angle Требуемый угол
     * \param [in] axis_num Номер ОСИ
     */
    virtual void set_angle(float angle, int axis_num) {};

    /**
     * \brief Задаёт диапазон изменения угла для определённой оси
     *
     * \param [in] start_angle Начальное значение угла
     * \param [in] stop_angle Конечное значение угла
     * \param [in] points Количество точек
     * \param [in] axis_num Номер оси
     */
    virtual void set_angle_range(float start_angle, float stop_angle, int points, int axis_num) {};

    /**
     * \brief Переход к следующей угловой точке
     *
     * \param [in] axis_num Номер оси ОПУ
     *
     * \return Если угловая точка находится в пределах диапазона изменения угла,
     * то возвращается ANGLE_MOVE_OK. Если угловая точка находится на границе
     * диапазона, то возвращает ANGLE_MOVE_BOUND.
     */
    virtual int next_angle(int axis_num) {return ANGLE_MOVE_BOUND;};

    /**
     * \brief Переход к предыдущей угловой точке
     *
     * \param [in] axis_num Номер оси ОПУ
     *
     * \return Если угловая точка находится в пределах диапазона изменения угла,
     * то возвращается ANGLE_MOVE_OK. Если угловая точка находится на границе
     * диапазона, то возвращает ANGLE_MOVE_BOUND.
     */
    virtual int prev_angle(int axis_num) {return ANGLE_MOVE_BOUND;};

    /**
     * \brief Устанавливает угловую точку, соответствующую началу
     * диапазона.
     *
     * \param [in] axis_num Номер оси ОПУ
     */
    virtual void move_to_start_angle(int axis_num) {};

    /**
     * \brief Устанавливает угловую точку, соответствующую концу
     * диапазона.
     *
     * \param [in] axis_num Номер оси ОПУ
     */
    virtual void move_to_stop_angle(int axis_num) {};

    /**
     * \brief Получает позицию требуемой оси
     *
     * \param [in] axis_num Ось, позицию которой требется узнать
     *
     * \return Позиция, в которой находится ось
     */
    virtual float get_pos(int axis_num) {return 0.0f;};

    /**
     * \brief Получает количество осей
     *
     * \return Количество осей
     */
    virtual int get_axes_count() {return 0;}
};

#endif //ANTESTL_BACKEND_RBD_DEVICE_HPP
