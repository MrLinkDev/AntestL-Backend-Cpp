#ifndef ANTESTL_BACKEND_UTILS_HPP
#define ANTESTL_BACKEND_UTILS_HPP

#include <string>

using namespace std;

void split(string *dest, int dest_size, const string source, char delimiter = ' ') {
    int pos = 0;

    for (char s : source) {
        if (s == delimiter) {
            ++pos;

            if (pos == dest_size) {
                return;
            }
        } else {
            dest[pos] += s;
        }
    }
}

int count(string target, char symbol) {
    int count = 0;

    for (char t : target) {
        if (t == symbol) {
            ++count;
        }
    }

    return count;
}

string lstrip(const string source, char symbol) {
    int pos = 0;
    string out{};

    while(source[pos] != symbol) {
        ++pos;
    }

    for (pos += 1; pos < source.length(); ++pos) {
        out.push_back(source[pos]);
    }

    return out;
}

#endif //ANTESTL_BACKEND_UTILS_HPP
