#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <sys/time.h>
#include <sstream>
#include <cmath>
#include <cstdarg>

#define LEVEL_INFO  0
#define LEVEL_ERROR 1
#define LEVEL_DEBUG 2

#define STR_TIME_BUFFER 128

#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"

typedef unsigned long long int size_t;

class Logger {
    static int log_level;

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

        char str_time[STR_TIME_BUFFER];
        strftime(str_time, STR_TIME_BUFFER, TIME_FORMAT, time_struct);

        char full_str_time[STR_TIME_BUFFER];
        sprintf(full_str_time, "%s.%d", str_time, milliseconds);

        return full_str_time;
    }

    static void print_to_console(const std::string &message) {
        printf("%s\n", message.c_str());
    }

public:
    static void init(int level) {
        log_level = level;
    }

    static void log(int level, const std::string& message) {
        std::string time = get_current_time();
        std::string level_tag{};

        switch (level) {
            case LEVEL_INFO:
                level_tag = " INFO";
                break;
            case LEVEL_ERROR:
                level_tag = "ERROR";
                break;
            case LEVEL_DEBUG:
                level_tag = "DEBUG";
                break;
            default:
                return;
        }

        std::stringstream formatted_message;
        formatted_message << time << " [" << level_tag << "] " << message;

        print_to_console(formatted_message.str());
        // TODO: print_to_file();
    }
};

typedef Logger logger;

#endif //LOGGER_HPP
