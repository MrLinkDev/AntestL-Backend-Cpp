#include "devices/vna/keysight_m9807a.hpp"
#include "devices/device_set.hpp"
#include "utils/string_utils.hpp"
#include "utils/test_json_requests.hpp"
#include "task_manager.hpp"
#include "socket/socket_server.hpp"

#define DEFAULT_TASK_PORT           5006
#define DEFAULT_DATA_PORT           5007

#define LOG_LEVEL_PARAM             "-log"
#define LOG_LEVEL_PARAM_SHORT       "-l"

#define LOG_LEVEL_TRACE             "trace"
#define LOG_LEVEL_DEBUG             "debug"
#define LOG_LEVEL_INFO              "info"
#define LOG_LEVEL_WARN              "warn"
#define LOG_LEVEL_ERROR             "error"

#define TASK_PORT_PARAM             "-task"
#define TASK_PORT_PARAM_SHORT       "-t"

#define DATA_PORT_PARAM             "-data"
#define DATA_PORT_PARAM_SHORT       "-d"

SocketServer task_server(DEFAULT_TASK_PORT);
SocketServer data_server(DEFAULT_DATA_PORT);

TaskManager task_manager();

void usage();

int main(int argc, char* argv[]) {
    logger::log(LEVEL_INFO, "Starting AntestL backend...");
    logger::set_log_level(LEVEL_INFO);

    for (int arg_pos = 1; arg_pos < argc; ++arg_pos) {
        if (strcmp(argv[arg_pos], LOG_LEVEL_PARAM) == 0 || strcmp(argv[arg_pos], LOG_LEVEL_PARAM_SHORT) == 0) {
            ++arg_pos;

            if (strcmp(argv[arg_pos], LOG_LEVEL_TRACE) == 0) {
                logger::set_log_level(LEVEL_TRACE);
            } else if (strcmp(argv[arg_pos], LOG_LEVEL_DEBUG) == 0) {
                logger::set_log_level(LEVEL_DEBUG);
            } else if (strcmp(argv[arg_pos], LOG_LEVEL_WARN) == 0) {
                logger::set_log_level(LEVEL_WARN);
            } else if (strcmp(argv[arg_pos], LOG_LEVEL_ERROR) == 0) {
                logger::set_log_level(LEVEL_ERROR);
            }
        } else if (strcmp(argv[arg_pos], TASK_PORT_PARAM) == 0 || strcmp(argv[arg_pos], TASK_PORT_PARAM_SHORT) == 0) {
            task_server.set_port(atoi(argv[++arg_pos]));
        } else if (strcmp(argv[arg_pos], DATA_PORT_PARAM) == 0 || strcmp(argv[arg_pos], DATA_PORT_PARAM_SHORT) == 0) {
            data_server.set_port(atoi(argv[++arg_pos]));
        } else {
            usage();
            exit(0);
        }
    }

    return 0;
}

void usage() {
    std::cout << "Usage was here" << std::endl;
}