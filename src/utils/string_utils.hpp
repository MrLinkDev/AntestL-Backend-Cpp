#ifndef ANTESTL_BACKEND_STRING_UTILS_HPP
#define ANTESTL_BACKEND_STRING_UTILS_HPP

#include <string>
#include <vector>

#define DEFAULT_DELIMITER       ' '

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

#endif //ANTESTL_BACKEND_STRING_UTILS_HPP
