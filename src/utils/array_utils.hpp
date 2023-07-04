/**
 * \file
 * \brief Файл, в котором определено пространство имён array_utils.
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_ARRAY_UTILS_HPP
#define ANTESTL_BACKEND_ARRAY_UTILS_HPP

#include <json.hpp>

/**
 * \brief Пространство имён, в котором определены методы для работы с массивами
 */
namespace array_utils {

    /**
     * \brief Функция, которая позволяет найти индекс требуемого элемента
     *
     * \param [in] target_array Указатель на массив, в котором будет производиться поиск элемента
     * \param [in] array_length Длина массива
     * \param [in] item Элемент массива, индекс которого требуется найти
     *
     * \return Если элемент был найден в массиве, то возвращается его индекс.
     * В противном случае, возвращается -1.
     */
    template <typename T>
    inline int index(T *target_array, int array_length, T item) {
        for (int pos = 0; pos < array_length; ++pos) {
            if (target_array[pos] == item) {
                return pos;
            }
        }

        return -1;
    }

    /**
     * \brief Функция для сравнения величины вложенности двух заданий
     *
     * \param [in] j1 Первое сравниваемое задание
     * \param [in] j2 Второе сравниваемое задание
     *
     * \return Возвращает true, если j1 < j2. В противном случае - false;
     */
    inline bool compare_nested(nlohmann::json j1, nlohmann::json j2) {
        return(j1["nested"].get<int>() < j2["nested"].get<int>());
    }
}

#endif //ANTESTL_BACKEND_ARRAY_UTILS_HPP
