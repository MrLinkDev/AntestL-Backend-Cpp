#include <condition_variable>

#include "socket/socket_server.hpp"
#include "task_manager.hpp"

#define DEFAULT_TASK_PORT           5006
#define DEFAULT_DATA_PORT           5007

#define TASK_SERVER_TAG             "TASK_S"
#define DATA_SERVER_TAG             "DATA_S"

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

SocketServer task_server(DEFAULT_TASK_PORT, TASK_SERVER_TAG);
SocketServer data_server(DEFAULT_DATA_PORT, DATA_SERVER_TAG);

TaskManager task_manager{};

std::jthread *task_thread;
std::jthread *data_thread;

std::condition_variable cv;
std::mutex mtx;

bool received = false;
bool processed = true;

json input_data{};
json data_buffer{};

bool stop_process = false;

void task_server_thread_f(std::stop_token s_token);
void data_server_thread_f(std::stop_token s_token);

void exit_event_handler(int signal_code);

void usage();

int main(int argc, char* argv[]) {
    signal(SIGINT, exit_event_handler);
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

    logger::log(LEVEL_INFO, "Starting AntestL backend...");

    while (!stop_process) {
        task_thread = new std::jthread{task_server_thread_f};
        std::this_thread::sleep_for(50ms);

        data_thread = new std::jthread{data_server_thread_f};

        task_thread->join();

        delete task_thread;
        delete data_thread;
    }

    return 0;
}

void task_server_thread_f(std::stop_token s_token) {
    if (task_server.create() != SOCKET_CREATED && !stop_process) {
        exit(1);
    }

    if (task_server.wait_client() != CLIENT_CONNECTED && !stop_process) {
        exit(1);
    }

    while (!s_token.stop_requested() || !stop_process) {
        try {
            input_data = std::move(json::parse(task_server.read_data()));
        } catch (const json::parse_error &err) {
            logger::log(LEVEL_ERROR, "Seems like input data cannot be parsed into json. Check input data!");

            std::this_thread::sleep_for(50ms);
            continue;
        }

        if (task_manager.received_stop_task(input_data)) {
            task_manager.request_stop();
            continue;
        }

        if (task_manager.received_disconnect_task(input_data)) {
            task_thread->request_stop();
            data_thread->request_stop();
        }

        std::unique_lock u_lk(mtx);
        cv.wait(u_lk, []{return processed;});

        data_buffer = std::move(input_data);

        received = true;
        processed = false;

        u_lk.unlock();
        cv.notify_one();
    }
}

void data_server_thread_f(std::stop_token s_token) {
    if (data_server.create() != SOCKET_CREATED && !stop_process) {
        exit(1);
    }

    if (data_server.wait_client() != CLIENT_CONNECTED && !stop_process) {
        exit(1);
    }

    std::string result{};

    while (!s_token.stop_requested() || !stop_process) {
        std::unique_lock u_lk(mtx);
        cv.wait(u_lk, []{return received;});

        result = std::move(task_manager.proceed(data_buffer));
        data_server.send_data(result);

        processed = true;
        received = false;

        u_lk.unlock();
        cv.notify_one();
    }

    data_server.close();
}

void exit_event_handler(int signal_code) {
    logger::log(LEVEL_INFO, "Stopping work...");

    stop_process = true;

    task_manager.request_stop();

    task_thread->request_stop();
    data_thread->request_stop();

    task_server.close();
    data_server.close();
}

void usage() {
    std::cout << "\n===== AntestL Backend =====\n" << std::endl;

    std::cout << "Usage:" << std::endl;
    std::cout << "-log   (-l) -- sets log level: [error, warn, info (default), debug, trace]" << std::endl;
    std::cout << "-task  (-t) -- sets task server port (default: 5006)" << std::endl;
    std::cout << "-data  (-d) -- sets data server port (default: 5007)" << std::endl;
}