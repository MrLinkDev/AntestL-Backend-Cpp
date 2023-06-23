#include "tesart_rbd.hpp"
#include "../../utils/exceptions.hpp"

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

void TesartRbd::init_params(size_t axis_count) {
    for (size_t i = 0; i < axis_count; ++i) {
        start_angle.push_back(0.0f);
        stop_angle.push_back(0.0f);

        angle_step.push_back(0.0f);
        current_angle.push_back(0.0f);

        points.push_back(0);
        current_point.push_back(0);
    }
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

    init_params(axes.size());
}

bool TesartRbd::is_connected() {
    bool connected = true;

    for (VisaDevice axis : axes) {
        connected &= axis.is_connected();
    }

    return connected;
}

bool TesartRbd::is_stopped(int axis_num) {
    int status_code = status(axis_num);

    return bool(status_code & BIT_IN_POS) and bool(!(status_code & BIT_MOVE_BLOCK));
}

void TesartRbd::move(float pos, int axis_num) {
    logger::log(LEVEL_DEBUG, "Axis {} pos = ", axis_num, pos);

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

void TesartRbd::set_angle(float angle, int axis_num) {
    this->start_angle[axis_num] = angle;
    this->stop_angle[axis_num] = angle;

    this->current_angle[axis_num] = angle;

    this->points[axis_num] = 1;
    this->current_point[axis_num] = 0;

    move(angle, axis_num);
}

void TesartRbd::set_angle_range(float start_angle, float stop_angle, int points, int axis_num) {
    this->start_angle[axis_num] = start_angle;
    this->stop_angle[axis_num] = stop_angle;

    this->current_angle[axis_num] = start_angle;

    angle_step[axis_num] =
            this->points[axis_num] <= 1 ?
            0 : (this->stop_angle[axis_num] - this->start_angle[axis_num]) / (this->points[axis_num] - 1);

    this->points[axis_num] = points;
    this->current_point[axis_num] = 0;
}

void TesartRbd::next_angle(int axis_num) {
    if (current_point[axis_num] == points[axis_num] - 1) {
        throw ANGLE_OUT_OF_BOUND;
    }

    current_angle[axis_num] += angle_step[axis_num];
    ++current_point[axis_num];

    move(current_angle[axis_num], axis_num);
}

void TesartRbd::prev_angle(int axis_num) {
    if (current_point[axis_num] == 0) {
        throw ANGLE_OUT_OF_BOUND;
    }

    current_angle[axis_num] -= angle_step[axis_num];
    --current_point[axis_num];

    move(current_angle[axis_num], axis_num);
}

void TesartRbd::move_to_start_angle(int axis_num) {
    current_angle[axis_num] = start_angle[axis_num];
    current_point[axis_num] = 0;

    move(current_angle[axis_num], axis_num);
}

void TesartRbd::move_to_stop_angle(int axis_num) {
    current_angle[axis_num] = stop_angle[axis_num];
    current_point[axis_num] = points[axis_num] - 1;

    move(current_angle[axis_num], axis_num);
}

float TesartRbd::get_pos(int axis_num) {
    std::string str_answer{};
    float answer{};

    axes[axis_num].clear();

    str_answer = axes[axis_num].send("PFB", true);
    answer = stof(str_answer) / SCALE;

    return answer;
}

int TesartRbd::get_axes_count() {
    return axes.size();
}
