/**
 * \file
 * \brief Библиотека, осуществляющая логирование
 *
 * \author Александр Горбунов
 * \date 3 июля 2023 года
 */

#ifndef ANTESTL_BACKEND_LOGGER_HPP
#define ANTESTL_BACKEND_LOGGER_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <cmath>
#include <format>

/// Уровень логирования, при котором отображаются только ошибки
#define LEVEL_ERROR 0
/// Уровень логирования, при котором отображаются ошибки и предупреждения
#define LEVEL_WARN  1
/// Уровень логирования, при котором отображаются ошибки, предупреждения и информационные сообщения
#define LEVEL_INFO  2
/// Уровень логирования, при котором отображаются ошибки, предупреждения, информационные сообщения и отладочные сообщения
#define LEVEL_DEBUG 3
/// Уровень логирования, при котором отображаеются все сообщения, включая сообщения трассировки
#define LEVEL_TRACE 4

/// Режим, при котором в консоль не выводятся цветные теги сообщений
#define NO_COLOR    false
/// Режим, при котором в консоль выводятся цветные теги сообщений
#define COLORED     true

/// Максимальная длина сообщения в консоли
#define MAX_MESSAGE_LENGTH      192

/// Размер буфера для строки времени
#define DEFAULT_BUFFER_SIZE     128
/// Шаблон строки времени
#define DEFAULT_TIME_FORMAT     "%Y-%m-%d %H:%M:%S"

/**
 * \brief Функция, которая снимает значение текущего времени и конвертирует его в строку с заданным шаблоном.
 *
 * @return Строка, отформатированная по заданному шаблону
 */
static std::string get_current_time() {
    struct timeval time_value{};
    struct tm *time_struct;

    gettimeofday(&time_value, NULL);
    int milliseconds = lrint(time_value.tv_usec / 1000.0);

    if (milliseconds >= 1000) {
        milliseconds -= 1000;
        time_value.tv_sec++;
    }

    time_t current_time = time_value.tv_sec;
    time_struct = localtime(&current_time);

    char str_time[DEFAULT_BUFFER_SIZE];
    strftime(str_time, DEFAULT_BUFFER_SIZE, DEFAULT_TIME_FORMAT, time_struct);

    return std::string(str_time) + std::format(".{:03}", milliseconds);
}

/**
 * \brief Класс Logger, в котором реализован набор методов для логирования
 */
class Logger {
    /// Уровень логирования. По-умолчанию задан максимальный уровень логирования.
    inline static int log_level = LEVEL_TRACE;
    /// Режим отображения сообщений. По-умолчанию задан режим отображения цветных тегов.
    inline static bool colored  = COLORED;

    /**
     * \brief Функция, возвращающая тег в зависимости от уровня логирования и от назначения тега.
     *
     * \param [in] level Уровень логирования
     * \param [in] for_file Требуется ли тег для записи в файл
     *
     * \return Требуемый тег
     */
    static std::string get_tag(int level, bool for_file) {
        switch (level) {
            case LEVEL_ERROR:
                return (!for_file & colored) ? "\033[38;5;196mERR\033[0m" : "ERR";
            case LEVEL_WARN:
                return (!for_file & colored) ? "\033[38;5;208mWRN\033[0m" : "WRN";
            case LEVEL_INFO:
                return (!for_file & colored) ? "\033[38;5;15mINF\033[0m" : "INF";
            case LEVEL_DEBUG:
                return (!for_file & colored) ? "\033[38;5;245mDBG\033[0m" : "DBG";
            case LEVEL_TRACE:
                return (!for_file & colored) ? "\033[38;5;240mTRC\033[0m" : "TRC";
            default:
                return std::string{};
        }
    }

    /**
     * \brief Функция, которая выводит сообщение в консоль.
     *
     * \param [in] message Сообщение, которое требуется вывести
     */
    static void print_to_console(const std::string &message) {
        std::cout << message << std::endl;
    }

    /**
     * \brief Функция, которая записывает сообщение в файл.
     *
     * \param [in] message Сообщение, которое требуется записать
     */
    static void print_to_file(const std::string &message) {
        // TODO: Добавить сохранение логов в файл
    }

public:
    /**
     * \brief Установка требуемого уровня логирования.
     *
     * \param [in] level Требуемый уровень логирования
     */
    static void set_log_level(int level) {
        log_level = level;
    }

    /**
     * \brief Установка требуемого вида тегов в консоли.
     *
     * \param [in] color_state Требуемый вид
     */
    static void set_color_state(bool color_state) {
        colored = color_state;
    }

    /**
     * \brief Функция, осуществляющая логирование
     *
     * Данная функция сравнивает текущий уровень логирования с уровнем входящего сообщения.
     * Если уровень входящего сообщения больше, чем текущий уровень, то сообщение игнорируется.
     *
     * \param [in] level Уровень сообщения
     * \param [in] message Сообщение
     *
     * **Пример**
     * \code
     * logger::set_log_level(LEVEL_INFO);
     *
     * logger::log(LEVEL_DEBUG, "debug message");   // Данное сообщение будет проигнорировано
     * logger::log(LEVEL_INFO), "info message");    // Данное сообщение будет выведено
     * \endcode
     */
    static void log(int level, const std::string &message) {
        if (level > log_level) return;

        std::string time = get_current_time();

        std::string log_level_tag_colored   = get_tag(level, false);
        std::string log_level_tag           = get_tag(level, true);

        std::string log_message_colored = time + " [" + log_level_tag_colored + "] " + message;
        std::string log_message         = time + " [" + log_level_tag + "] " + message;

        if (log_message_colored.length() > MAX_MESSAGE_LENGTH) {
            log_message_colored = log_message.substr(0, MAX_MESSAGE_LENGTH) + "...";
        }

        print_to_console(log_message_colored);
        print_to_file(log_message);
    }

    /**
     * \brief Функция, осуществляющая логирование
     *
     * Данная функция сравнивает текущий уровень логирования с уровнем входящего сообщения.
     * Если уровень входящего сообщения больше, чем текущий уровень, то сообщение игнорируется.
     * Позволяет передать строку формата и аргументы для этой строки.
     *
     * \param [in] level Уровень сообщения
     * \param [in] fmt Формат сообщения
     * \param [in] args Аргументы для сообщения
     *
     * **Пример**
     * \code
     * logger::set_log_level(LEVEL_WARN);
     *
     * logger::log(LEVEL_INFO, "{} ({}) created", socket_tag, socket_port);         // Сообщение будет проигнорировано
     * logger::log(LEVEL_ERROR, "{} ({}) no clients", socket_tag, socket_port);     // Сообщение будет выведено
     * \endcode
     */
    template <typename... T>
    static void log(int level, const std::string &fmt, T&&... args) {
        if (level > log_level) return;

        std::string time = get_current_time();

        std::string log_level_tag_colored   = get_tag(level, false);
        std::string log_level_tag           = get_tag(level, true);

        std::string log_message_colored = time + " [" + log_level_tag_colored + "] ";
        log_message_colored += std::vformat(fmt, std::make_format_args(args...));

        std::string log_message = time + " [" + log_level_tag + "] ";
        log_message += std::vformat(fmt, std::make_format_args(args...));

        if (log_message_colored.length() > MAX_MESSAGE_LENGTH) {
            log_message_colored = log_message_colored.substr(0, MAX_MESSAGE_LENGTH) + "...";
        }

        print_to_console(log_message_colored);
        print_to_file(log_message);
    }

};

typedef Logger logger;

#endif //ANTESTL_BACKEND_LOGGER_HPP
