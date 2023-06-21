#include "tesart_rbd.hpp"

int TesartRbd::status(int axis_num) {
    std::string str_answer{};
    int answer{};

    axes[axis_num].clear();

    try {
        str_answer = axes[axis_num].send("TRJSTAT", true);
        str_answer = string_utils::lstrip(str_answer, 'H');

        answer = stoi(str_answer);
    } catch (std::invalid_argument inv_arg) {
        str_answer = axes[axis_num].send("TRJSTAT", true);
        str_answer = string_utils::lstrip(str_answer, 'H');

        answer = stoi(str_answer);
    }

    return answer;
}

int TesartRbd::status(VisaDevice *axis) {
    std::string str_answer{};
    int answer{};

    axis->clear();

    try {
        str_answer = axis->send("TRJSTAT", true);
        str_answer = string_utils::lstrip(str_answer, 'H');

        answer = stoi(str_answer);
    } catch (std::invalid_argument inv_arg) {
        str_answer = axis->send("TRJSTAT", true);
        str_answer = string_utils::lstrip(str_answer, 'H');

        answer = stoi(str_answer);
    }

    return answer;
}

TesartRbd::TesartRbd(const std::string &device_addresses) {
    std::vector<std::string> address_list = string_utils::split(device_addresses, ADDRESS_DELIMITER);

    for (const std::string &address : address_list) {
        VisaDevice axis(address);

        axis.send("PROMPT 0");
        axis.send("CLRFAULT");
        axis.send("EN");

        axis.clear();
        std::this_thread::sleep_for(TESART_RBD_SLEEP_TIME_INIT);

        if (!(status(&axis) & BIT_REF_SET)) {
            axis.send("MH");
        }

        axes.push_back(axis);
    }
}

bool TesartRbd::is_stopped(int axis_num) {
    int status_code = status(axis_num);

    return bool(status_code & BIT_IN_POS) and bool(!(status_code & BIT_MOVE_BLOCK));
}

void TesartRbd::move(float pos, int axis_num) {
    logger::log(LEVEL_INFO, "Axis {} pos = ", axis_num, pos);

    axes[axis_num].send(
            "ORDER 0 {} {} 8192 {} {} 0 -1 0 0",
            int(pos * SCALE), velocity, acceleration, acceleration);
    axes[axis_num].send("MOVE 0");
}

void TesartRbd::stop() {
    for (VisaDevice axis : axes) {
        axis.send("STOP");
    }
}

float TesartRbd::get_pos(int axis_num) {
    std::string str_answer{};
    float answer{};

    axes[axis_num].clear();

    str_answer = axes[axis_num].send("PFB", true);
    answer = stof(str_answer) / SCALE;

    return answer;
}
