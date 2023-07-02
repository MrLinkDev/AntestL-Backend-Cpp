#ifndef ANTESTL_BACKEND_ARRAY_UTILS_HPP
#define ANTESTL_BACKEND_ARRAY_UTILS_HPP

#include <json.hpp>

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

    inline bool compare_nested(nlohmann::json j1, nlohmann::json j2) {
        return(j1["nested"].get<int>() < j2["nested"].get<int>());
    }
}

#endif //ANTESTL_BACKEND_ARRAY_UTILS_HPP
