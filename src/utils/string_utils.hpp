/**
 * \file
 * \brief Заголовочный файл, в котором определено пространство имён string_utils
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

#ifndef ANTESTL_BACKEND_STRING_UTILS_HPP
#define ANTESTL_BACKEND_STRING_UTILS_HPP

#include <string>
#include <vector>

/// Стандартный разделитель для функции split()
#define DEFAULT_DELIMITER       ' '

/**
 * \brief Пространство имён, в котором определены вспомогательные методы для работы со строками
 */
namespace string_utils {

    /**
     * \brief Подсчёт количества символов в строке
     *
     * \param [in] target Строка, в которой осуществляется поиск требуемого символа
     * \param [in] symbol Требуемый символ
     *
     * \return Количество символов
     *
     * **Пример**
     * \code
     * std::string a = "Hello world!";
     *
     * std::cout << string_utils::count(a, 'l') << endl;    // Будет выведено число 3
     * std::cout << string_utils::count(a, 'H') << endl;    // Будет выведено число 1
     * std::cout << string_utils::count(a, '?') << endl;    // Будет выведено число 0
     * \endcode
     */
    inline int count(const std::string &target, char symbol) {
        int count = 0;

        for (char t : target) {
            if (t == symbol) {
                ++count;
            }
        }

        return count;
    }

    /**
     * \brief Деление строки на вектор строк по заданному делителю
     *
     * \param [in] source Строка, которую требуется разделить
     * \param [in] delimiter Делитель
     *
     * \return Массив строк, в которые не будет включен делитель
     *
     * **Пример**
     * \code
     * std::string a = "address1;address2;address3";
     * std::vector<std::string> b = string_utils::split(a, ';');    // Вектор содержит три элемента: "address1", "address2" и "address3"
     * \endcode
     */
    inline std::vector<std::string> split(const std::string &source, char delimiter = DEFAULT_DELIMITER) {
        std::vector<std::string> out{};
        std::string cache{};

        for(char symbol : source) {
            if (symbol != delimiter) {
                cache.push_back(symbol);
            } else {
                out.push_back(cache);
                cache.clear();
            }
        }

        out.push_back(cache);
        cache.clear();

        return out;
    }

    /**
     * \brief Делит строку на две части по требуемому символу и возвращает содержимое правой части
     *
     * \param [in] source Строка, которую требуется разделить
     * \param [in] strip_symbol Символ, по которому осуществляется деление
     *
     * \return Содержимое справа от символа деления
     *
     * **Пример**
     * \code
     * std::string a = "Hello world!";
     * std::cout << string_utils::lstrip(a, ' ') << std::endl;  // Будет выведена строка "456"
     * \endcode
     */
    inline std::string lstrip(const std::string &source, char strip_symbol) {
        return source.substr(source.find(strip_symbol) + 1, std::string::npos);
    }

    /**
     * \brief Делит строку на две части по требуемому символу и возвращает содержимое левой части
     *
     * \param [in] source Строка, которую требуется разделить
     * \param [in] strip_symbol Символ, по которому осуществляется деление
     *
     * \return Содержимое слева от символа деления
     *
     * **Пример**
     * \code
     * std::string a = "Hello world!";
     * std::cout << string_utils::lstrip(a, ' ') << std::endl;  // Будет выведена строка "123"
     * \endcode
     */
    inline std::string rstrip(const std::string &source, char strip_symbol) {
        return source.substr(0, source.rfind(strip_symbol));
    }
}

#endif //ANTESTL_BACKEND_STRING_UTILS_HPP
