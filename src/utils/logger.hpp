#ifndef ANTESTL_BACKEND_LOGGER_HPP
#define ANTESTL_BACKEND_LOGGER_HPP

#include <string>
#include <iostream>
#include <sstream>

#include "utils.hpp"

#define LEVEL_ERROR 0
#define LEVEL_WARN  1
#define LEVEL_INFO  2
#define LEVEL_DEBUG 3

#define NO_COLOR    false
#define COLORED     true

#define MAX_MESSAGE_LENGTH  128

class Logger {

    inline static int log_level = LEVEL_DEBUG;
    inline static bool colored  = COLORED;

    static std::string get_tag(int level, bool for_file) {
        switch (level) {
            case LEVEL_ERROR:
                return (!for_file & colored) ? "\033[1;31mERR\033[0m" : "ERR";
            case LEVEL_WARN:
                return (!for_file & colored) ? "\033[1;33mWRN\033[0m" : "WRN";
            case LEVEL_INFO:
                return (!for_file & colored) ? "\033[1;97mINF\033[0m" : "INF";
            case LEVEL_DEBUG:
                return (!for_file & colored) ? "\033[37mDBG\033[0m" : "DBG";
            default:
                return std::string{};
        }
    }

    static void print_to_console(const std::string &message) {
        std::cout << message << std::endl;
    }

    static void print_to_file(const std::string &message) {
        // TODO: Добавить сохранение логов в файл
    }

public:
    static void set_log_level(int level) {
        log_level = level;
    }

    static void set_color_state(bool color_state) {
        colored = color_state;
    }

    static void log(int level, const std::string &message) {
        if (level > log_level) return;

        std::string time = time_utils::get_current_time();

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

    template <typename... T>
    static void log(int level, const std::string &fmt, T&&... args) {
        if (level > log_level) return;

        std::string time = time_utils::get_current_time();

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
