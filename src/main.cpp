/**
 * \file
 * \brief Главный файл приложения
 *
 * \author Александр Горбунов
 * \date 3 июля 2023
 */

/**
 *
 */

#include <condition_variable>

#include "socket/socket_server.hpp"
#include "task_manager.hpp"

/// Версия AntestL Backend
#define VERSION     "1.0.7"

/// Стандартный порт для входящих заданий
#define DEFAULT_TASK_PORT           5006
/// Стандартный порт для исходящих данных
#define DEFAULT_DATA_PORT           5007

/// Тег для сокета заданий
#define TASK_SERVER_TAG             "TASK_S"
/// Тег для сокета данных
#define DATA_SERVER_TAG             "DATA_S"

/// Параметр изменения уровня логирования
#define LOG_LEVEL_PARAM             "-log"
/// Параметр изменения уровня логирования (укороченный)
#define LOG_LEVEL_PARAM_SHORT       "-l"

/// Аргумент для изменения логирования (LOG_LEVEL = LEVEL_TRACE)
#define LOG_LEVEL_TRACE             "trace"
/// Аргумент для изменения логирования (LOG_LEVEL = LEVEL_DEBUG)
#define LOG_LEVEL_DEBUG             "debug"
/// Аргумент для изменения логирования (LOG_LEVEL = LEVEL_WARN)
#define LOG_LEVEL_WARN              "warn"
/// Аргумент для изменения логирования (LOG_LEVEL = LEVEL_ERROR)
#define LOG_LEVEL_ERROR             "error"

/// Параметр изменения порта для входящих заданий
#define TASK_PORT_PARAM             "-task"
/// Параметр изменения порта для входящих заданий (укороченный)
#define TASK_PORT_PARAM_SHORT       "-t"

/// Параметр изменения порта для исходящих данных
#define DATA_PORT_PARAM             "-data"
/// Параметр изменения порта для исходящих данных (укороченный)
#define DATA_PORT_PARAM_SHORT       "-d"

/// Объект сокета входящих заданий
SocketServer task_server(DEFAULT_TASK_PORT, TASK_SERVER_TAG);
/// Объект сокета исходящих данных
SocketServer data_server(DEFAULT_DATA_PORT, DATA_SERVER_TAG);

/// Объект менеджера заданий
TaskManager task_manager{};

/// Поток для приёма входящих заданий
std::jthread *task_thread;
/// Поток для обработки и отправки данных
std::jthread *data_thread;

/// Объект, необходимый для организации синхронной обработки данных двумя потоками
std::condition_variable cv;
/// Мьютекс, необходимый для организации синхронной обработки данных двумя потоками
std::mutex mtx;

/// Флаг, показывающий, что данные бьыли приняты от клиента
bool received = false;
/// Флаг, показывающий, что данные бьыли обработаны и отправлены клиенту
bool processed = true;

/// Буфер для входящих данных
json input_data{};
/// Буфер для данных, переданных на обработку
json data_buffer{};

/// Флаг, показывающий, что было нажато сочетание клавиш Ctrl+C
bool stop_process = false;

/// Флаг, показывающий, что требуется ожидание нового клиента
bool wait_another = false;

void task_server_thread_f(std::stop_token s_token);
void data_server_thread_f(std::stop_token s_token);

void exit_event_handler(int signal_code);

void usage();

int main(int argc, char* argv[]) {
    signal(SIGINT, exit_event_handler);

    logger::set_log_level(LEVEL_INFO);
    logger::set_color_state(NO_COLOR);

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

    logger::log(LEVEL_INFO, "Starting AntestL Backend (v{})", VERSION);

    while (!stop_process) {
        if (wait_another) {
            wait_another = false;
        }

        task_thread = new std::jthread{task_server_thread_f};
        std::this_thread::sleep_for(50ms);

        data_thread = new std::jthread{data_server_thread_f};

        task_thread->join();

        delete task_thread;
        delete data_thread;
    }

    return 0;
}

/**
 * \brief Приём данных от клиента и передача их потоку обработки
 *
 * \param [in] s_token Токен, показывающий, что была запрошена остановка потока
 */
void task_server_thread_f(std::stop_token s_token) {
    if (task_server.create() != SOCKET_CREATED && !stop_process) {
        exit(1);
    }

    if (task_server.wait_client() != CLIENT_CONNECTED && !stop_process) {
        exit(1);
    }

    while (!s_token.stop_requested() && !stop_process && !wait_another) {
        try {
            input_data = std::move(json::parse(task_server.read_data()));
        } catch (const json::parse_error &err) {
            if (!task_server.is_connected()) {
                task_thread->request_stop();
                data_thread->request_stop();

                wait_another = true;
            } else {
                logger::log(LEVEL_ERROR, "Seems like input data cannot be parsed into json. Check input data!");

                std::this_thread::sleep_for(50ms);
                continue;
            }
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

    task_server.close();
}

/**
 * \brief Обработка принятых данных и отправка результата обработки клиенту
 *
 * \param [in] s_token Токен, показывающий, что была запрошена остановка потока
 */
void data_server_thread_f(std::stop_token s_token) {
    if (data_server.create() != SOCKET_CREATED && !stop_process) {
        exit(1);
    }

    if (data_server.wait_client() != CLIENT_CONNECTED && !stop_process) {
        exit(1);
    }

    std::string result{};

    while (!s_token.stop_requested() && !stop_process && !wait_another) {
        std::unique_lock u_lk(mtx);
        cv.wait(u_lk, []{return received;});

        if (data_server.is_connected() && !wait_another) {
            result = std::move(task_manager.proceed(data_buffer));
            data_server.send_data(result);
        }

        processed = true;
        received = false;

        u_lk.unlock();
        cv.notify_one();
    }

    data_server.close();
}

/**
 * \brief Функция, вызываемая при нажатии сочетания клавиш Ctrl+C
 *
 * \param [in] signal_code Код сигнала
 */
void exit_event_handler(int signal_code) {
    logger::log(LEVEL_INFO, "Stopping work...");

    stop_process = true;

    task_server.close();
    data_server.close();

    task_manager.request_stop();

    task_thread->request_stop();
    data_thread->request_stop();
}

/**
 * \brief Функция, выводящая справку на экран
 */
void usage() {
    std::cout << "\n===== AntestL Backend =====\n" << std::endl;

    std::cout << "Usage:" << std::endl;
    std::cout << "-log   (-l) -- sets log level: [error, warn, info (default), debug, trace]" << std::endl;
    std::cout << "-task  (-t) -- sets task server port (default: 5006)" << std::endl;
    std::cout << "-data  (-d) -- sets data server port (default: 5007)" << std::endl;
}