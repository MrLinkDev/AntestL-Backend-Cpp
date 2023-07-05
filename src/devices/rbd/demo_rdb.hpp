/**
 * \file
 * \brief Заголовочный файл, в котором реализован класс DemoRbd
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_DEMO_RDB_HPP
#define ANTESTL_BACKEND_DEMO_RDB_HPP

#include <thread>
#include "rbd_device.hpp"
#include "../../utils/exceptions.hpp"
#include "../../utils/string_utils.hpp"

/**
 * \brief Класс DemoRbd, который является эмулятором ОПУ
 *
 * В данном классе переопределены методы класса RbdDevice таким образом,
 * чтобы можно было работать с ОПУ без подключения к реальному устройству
 */
class DemoRbd : public RbdDevice {
protected:
    /// Количество осей
    int axis_count = 0;
    /// Текущие позиции осей
    std::vector<float> axis_pos{};

public:
    /**
     * \brief Конструктор, в который передаётся адрес (или список одресов,
     * разделённых символом ';')
     *
     * \param [in] device_addresses адрес оси (список адресов осей)
     */
    DemoRbd(const std::string &device_addresses):RbdDevice() {
        axis_count = string_utils::count(device_addresses, ADDRESS_DELIMITER) + 1;

        for (int i = 0; i < axis_count; ++i) {
            axis_pos.push_back(0);
        }

        init_params(axis_count);
    }

    /**
     * \brief Метод, возвращающий значение флага connected
     *
     * \warning В эмуляторе всегда возвращает true
     */
    bool is_connected() override {
        return true;
    };

    /**
     * \brief Поворачивает ось до тех пор, пока она не достигнет требуемого угла
     *
     * \param [in] pos Требуемый угол
     * \param [in] axis_num Номер оси
     */
    void move(float pos, int axis_num) override {
        axis_pos[axis_num] = pos;
    };

    /**
     * \brief Поворачивает ось ОПУ на требуемый угол
     *
     * \param [in] angle Требуемый угол
     * \param [in] axis_num Номер ОСИ
     */
    void set_angle(float angle, int axis_num) override {
        this->start_angle[axis_num] = angle;
        this->stop_angle[axis_num] = angle;

        this->current_angle[axis_num] = angle;

        this->points[axis_num] = 1;
        this->current_point[axis_num] = 0;

        move(angle, axis_num);
    };

    /**
     * \brief Задаёт диапазон изменения угла для определённой оси
     *
     * \param [in] start_angle Начальное значение угла
     * \param [in] stop_angle Конечное значение угла
     * \param [in] points Количество точек
     * \param [in] axis_num Номер оси\
     */
    void set_angle_range(float start_angle, float stop_angle, int points, int axis_num) override {
        this->start_angle[axis_num] = start_angle;
        this->stop_angle[axis_num] = stop_angle;
        this->points[axis_num] = points;

        angle_step[axis_num] =
                this->points[axis_num] <= 1 ?
                0 : (this->stop_angle[axis_num] - this->start_angle[axis_num]) / (this->points[axis_num] - 1);

        this->current_angle[axis_num] = start_angle;
        this->current_point[axis_num] = 0;
    };

    /**
     * \brief Переход к следующей угловой точке
     *
     * \param [in] axis_num Номер оси ОПУ
     *
     * \return Если угловая точка находится в пределах диапазона изменения угла,
     * то возвращается ANGLE_MOVE_OK. Если угловая точка находится на границе
     * диапазона, то возвращает ANGLE_MOVE_BOUND.
     */
    int next_angle(int axis_num) override {
        if (current_point[axis_num] == points[axis_num] - 1) {
            return ANGLE_MOVE_BOUND;
        }

        current_angle[axis_num] += angle_step[axis_num];
        ++current_point[axis_num];

        move(current_angle[axis_num], axis_num);

        return ANGLE_MOVE_OK;
    };

    /**
     * \brief Переход к предыдущей угловой точке
     *
     * \param [in] axis_num Номер оси ОПУ
     *
     * \return Если угловая точка находится в пределах диапазона изменения угла,
     * то возвращается ANGLE_MOVE_OK. Если угловая точка находится на границе
     * диапазона, то возвращает ANGLE_MOVE_BOUND.
     */
    int prev_angle(int axis_num) override {
        if (current_point[axis_num] == 0) {
            return ANGLE_MOVE_BOUND;
        }

        current_angle[axis_num] -= angle_step[axis_num];
        --current_point[axis_num];

        return ANGLE_MOVE_OK;
    };

    /**
     * \brief Устанавливает угловую точку, соответствующую началу
     * диапазона.
     *
     * \param [in] axis_num Номер оси ОПУ
     */
    void move_to_start_angle(int axis_num) override {
        current_angle[axis_num] = start_angle[axis_num];
        current_point[axis_num] = 0;

        move(current_angle[axis_num], axis_num);
    };

    /**
     * \brief Устанавливает угловую точку, соответствующую концу
     * диапазона.
     *
     * \param [in] axis_num Номер оси ОПУ
     */
    void move_to_stop_angle(int axis_num) override {
        current_angle[axis_num] = stop_angle[axis_num];
        current_point[axis_num] = points[axis_num] - 1;

        move(current_angle[axis_num], axis_num);
    };

    /**
     * \brief Получает позицию требуемой оси
     *
     * \param [in] axis_num Ось, позицию которой требется узнать
     *
     * \return Позиция, в которой находится ось
     */
    float get_pos(int axis_num) override {
        return axis_pos[axis_num];
    };

    /**
     * \brief Получает количество осей
     *
     * \return Количество осей
     */
    int get_axes_count() override {
        return axis_count;
    }
};

#endif //ANTESTL_BACKEND_DEMO_RDB_HPP
