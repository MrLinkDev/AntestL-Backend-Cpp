#include <iostream>
#include "utils/utils.hpp"
#include "utils/logger.hpp"

using namespace std;

int main() {
    logger::set_log_level(LEVEL_DEBUG);

    logger::log(LEVEL_ERROR, "Error msg");
    logger::log(LEVEL_INFO, "Info msg");
    logger::log(LEVEL_DEBUG, "Debug msg");

    logger::log(LEVEL_DEBUG, "{} {}", 123, 123);

    return 0;
}
