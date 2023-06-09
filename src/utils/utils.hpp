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

using namespace std;

namespace string_utils {

    int count(const string &target, char symbol) {
        int count = 0;

        for (char t : target) {
            if (t == symbol) {
                ++count;
            }
        }

        return count;
    }

    vector<string> split(const string &source, char delimiter = DEFAULT_DELIMITER) {
        vector<string> out{};
        string cache{};

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

    string lstrip(const string &source, char strip_symbol) {
        return source.substr(source.find(strip_symbol) + 1, string::npos);
    }

    string rstrip(const string &source, char strip_symbol) {
        return source.substr(0, source.rfind(strip_symbol));
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

        return string(str_time) + format(".{:03}", milliseconds);
    }
}

#endif //ANTESTL_BACKEND_UTILS_HPP
