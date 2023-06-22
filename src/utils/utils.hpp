#ifndef ANTESTL_BACKEND_UTILS_HPP
#define ANTESTL_BACKEND_UTILS_HPP

#include <string>
#include <vector>

#include <sys/time.h>
#include <cmath>
#include <format>

#define DEFAULT_DELIMITER       ' '

#define DEFAULT_BUFFER_SIZE     128
#define DEFAULT_TIME_FORMAT     "%Y-%m-%d %H:%M:%S"

namespace string_utils {

    inline int count(const std::string &target, char symbol) {
        int count = 0;

        for (char t : target) {
            if (t == symbol) {
                ++count;
            }
        }

        return count;
    }

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

    inline std::string lstrip(const std::string &source, char strip_symbol) {
        return source.substr(source.find(strip_symbol) + 1, std::string::npos);
    }

    inline std::string rstrip(const std::string &source, char strip_symbol) {
        return source.substr(0, source.rfind(strip_symbol));
    }

    inline void join(std::string *source, std::string item) {
        source->append(item);
    }

    inline void join(std::string *source, std::string item, char delimiter) {
        source->push_back(delimiter);
        source->append(item);
    }
}

namespace time_utils {

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
}

namespace array_utils {

    template <typename T>
    inline int index(T *target_array, int array_length, T item) {
        for (int pos = 0; pos < array_length; ++pos) {
            if (target_array[pos] == item) {
                return pos;
            }
        }

        return -1;
    }
}

#endif //ANTESTL_BACKEND_UTILS_HPP
