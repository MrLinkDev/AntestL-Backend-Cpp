#ifndef ANTESTL_BACKEND_LOGGER_HPP
#define ANTESTL_BACKEND_LOGGER_HPP

#include <string>
#include <iostream>
#include <sstream>

#include "utils.hpp"

#define LEVEL_ERROR 0
#define LEVEL_INFO  1
#define LEVEL_DEBUG 2

using namespace std;

typedef unsigned long long int size_t;

class Logger {

    inline static int log_level = LEVEL_DEBUG;

    static string get_tag(int level) {
        switch (level) {
            case LEVEL_ERROR:
                return "ERROR";
            case LEVEL_INFO:
                return " INFO";
            case LEVEL_DEBUG:
                return "DEBUG";
            default:
                return "";
        }
    }

    static void print_to_console(const string &message) {
        cout << message << endl;
    }

    static void print_to_file(const string &message) {
        // TODO: Добавить сохранение логов в файл
    }

public:
    static void set_log_level(int level) {
        log_level = level;
    }

    static void log(int level, const string &message) {
        if (level > log_level) return;

        string time = time_utils::get_current_time();
        string log_level_tag = get_tag(level);

        string log_message = time + " [" + log_level_tag + "] " + message;

        print_to_console(log_message);
        print_to_file(log_message);
    }

    template <typename... T>
    static void log(int level, const string &fmt, T&&... args) {
        if (level > log_level) return;

        string time = time_utils::get_current_time();
        string log_level_tag = get_tag(level);

        string log_message = time + " [" + log_level_tag + "] ";
        log_message += vformat(fmt, make_format_args(args...));

        print_to_console(log_message);
        print_to_file(log_message);
    }

};

typedef Logger logger;

#endif //ANTESTL_BACKEND_LOGGER_HPP
