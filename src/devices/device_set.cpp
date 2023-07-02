#include "device_set.hpp"
#include "rbd/demo_rdb.hpp"

bool DeviceSet::connect(int device_type, std::string device_model, const std::string &device_address) {
    std::transform(device_model.begin(), device_model.end(), device_model.begin(), ::toupper);

    try {
        switch (device_type) {
            case DEVICE_VNA:
                logger::log(LEVEL_TRACE, "Connecting to VNA");

                if (device_model == "M9807A") {
                    vna = new KeysightM9807A(device_address);
                    vna->preset();
                } else {
                    return false;
                }

                return vna->is_connected();
            case DEVICE_GEN:
                logger::log(LEVEL_TRACE, "Connecting to external gen");
                ext_gen = new KeysightGen(device_address);

                return ext_gen->is_connected();
            case DEVICE_RBD:
                logger::log(LEVEL_TRACE, "Connecting to RBD");

                if (device_model == "TESART_RBD") {
                    rbd = new TesartRbd(device_address);
                } else if (device_model == "UPKB_RBD") {
                    //TODO: Добавить инициализацию объекта
                } else if (device_model == "DEMO_RBD") {
                    rbd = new DemoRbd(device_address);
                } else {
                    return false;
                }

                return rbd->is_connected();
            default:
                return false;
        }
    } catch (const antestl_exception &exception) {
        return false;
    }
}

void DeviceSet::disconnect() {
    delete vna;
    delete ext_gen;
    delete rbd;
}

bool DeviceSet::configure(int meas_type, float rbw, int source_port, bool using_ext_gen) {
    try {
        vna->full_preset();
        logger::log(LEVEL_TRACE, "Made full preset");

        vna->init_channel();
        logger::log(LEVEL_TRACE, "Default channel initialized");

        vna->configure(meas_type, rbw, source_port);

        this->meas_type = meas_type;
        this->using_ext_gen = using_ext_gen;
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't configure VNA");
        return false;
    }

    traces_configured = false;

    logger::log(LEVEL_DEBUG, "VNA configured");
    return true;
}

bool DeviceSet::set_power(float power) {
    try {
        if (using_ext_gen) {
            ext_gen->set_power(power);
            logger::log(LEVEL_TRACE, "External gen power = {}", power);
        } else {
            vna->set_power(power);
            logger::log(LEVEL_DEBUG, "VNA power = {}", power);
        }
    } catch (const antestl_exception &exception) {
        if (using_ext_gen) {
            logger::log(LEVEL_ERROR, "Can't change power on external generator");
        } else {
            logger::log(LEVEL_ERROR, "Can't change power on VNA");
        }

        return false;
    }

    return true;
}

bool DeviceSet::set_freq(double freq) {
    try {
        if (using_ext_gen) {
            ext_gen->set_freq(freq);
            logger::log(LEVEL_DEBUG, "External gen frequency = {}", freq);
        } else {
            vna->set_freq(freq);
            logger::log(LEVEL_DEBUG, "VNA frequency = {}", freq);
        }
    } catch (const antestl_exception &exception) {
        if (using_ext_gen) {
            logger::log(LEVEL_ERROR, "Can't change frequency on external generator");
        } else {
            logger::log(LEVEL_ERROR, "Can't change frequency on VNA");
        }

        return false;
    }

    return true;
}

bool DeviceSet::set_freq_range(double start_freq, double stop_freq, int points) {
    try {
        if (using_ext_gen) {
            ext_gen->set_freq_range(start_freq, stop_freq, points);
            logger::log(LEVEL_DEBUG, "External gen frequency range = [{}; {}] ({} points)", start_freq, stop_freq, points);
        } else {
            vna->set_freq_range(start_freq, stop_freq, points);
            logger::log(LEVEL_DEBUG, "VNA frequency range = [{}; {}] ({} points)", start_freq, stop_freq, points);
        }
    } catch (const antestl_exception &exception) {
        if (using_ext_gen) {
            logger::log(LEVEL_ERROR, "Can't change frequency range on external generator");
        } else {
            logger::log(LEVEL_ERROR, "Can't change frequency range on VNA");
        }

        return false;
    }

    return true;
}

int DeviceSet::next_freq() {
    try {
        return ext_gen->next_freq();
    } catch (const antestl_exception &exception) {
        return -1;
    }
}

int DeviceSet::prev_freq() {
    try {
        return ext_gen->prev_freq();
    } catch (const antestl_exception &exception) {
        return -1;
    }
}

bool DeviceSet::move_to_start_freq() {
    if (using_ext_gen) {
        try {
            ext_gen->move_to_start_freq();
        } catch (const antestl_exception &exception) {
            logger::log(LEVEL_ERROR, "Can't set frequency on external gen");
            return false;
        }
    }

    return true;
}

double DeviceSet::get_current_freq() {
    if (ext_gen != nullptr && ext_gen->is_connected()) {
        try {
            return ext_gen->get_current_freq();
        } catch (const antestl_exception &exception) {
            logger::log(LEVEL_ERROR, "Can't get current frequency from gen");
            return 0.0;
        }
    } else {
        return 0.0;
    }
}

std::vector<double> DeviceSet::get_freq_list() {
    std::vector<double> freq_list{};

    for (int pos = 0; pos < vna->get_points(); ++pos) {
        freq_list.push_back(vna->get_start_freq() + pos * vna->get_freq_step());
    }

    return freq_list;
}

bool DeviceSet::set_angle(float angle, int axis_num) {
    try {
        rbd->set_angle(angle, axis_num);
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't set angle on RBD");
        return false;
    }

    logger::log(LEVEL_DEBUG, "RBD (axis {}): angle = {}", axis_num, angle);
    return true;
}

bool DeviceSet::set_angle_range(float start_angle, float stop_angle, int points, int axis_num) {
    try {
        rbd->set_angle_range(start_angle, stop_angle, points, axis_num);
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't set angle range on RBD");
        return false;
    }

    logger::log(LEVEL_DEBUG, "RBD (axis {}): angle range = [{}; {}] ({} points)", axis_num, start_angle, stop_angle, points);
    return true;
}

int DeviceSet::next_angle(int axis_num) {
    try {
        return rbd->next_angle(axis_num);
    } catch (const antestl_exception &exception) {
        return -1;
    }
}

int DeviceSet::prev_angle(int axis_num) {
    try {
        return rbd->prev_angle(axis_num);
    } catch (const antestl_exception &exception) {
        return -1;
    }
}

bool DeviceSet::move_to_start_angle(int axis_num) {
    try {
        rbd->move_to_start_angle(axis_num);
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't set angle on RBD (axis {})", axis_num);
        return false;
    }

    return true;
}

std::string DeviceSet::get_current_angles() {
    std::string angle_list{};

    if (rbd != nullptr && rbd->is_connected()) {
        for (int axis = 0; axis < rbd->get_axes_count(); ++axis) {
            if (axis == 0) {
                angle_list = std::to_string(rbd->get_pos(axis));
            } else {
                angle_list += "," + std::to_string(rbd->get_pos(axis));
            }
        }
    }

    return angle_list;
}

bool DeviceSet::set_path(std::vector<int> path_list) {
    try {
        vna->set_path(std::move(path_list));
    } catch (const antestl_exception &exception) {
        logger::log(LEVEL_ERROR, "Can't change switch paths on VNA");
        return false;
    }

    logger::log(LEVEL_DEBUG, "Changed switch paths on VNA");
    return true;
}

int DeviceSet::get_vna_switch_module_count()  {
    if (vna != nullptr && vna->is_connected()) {
        return vna->get_switch_module_count();
    } else {
        return 0;
    }
}

bool DeviceSet::is_using_ext_gen() const {
    return using_ext_gen;
}

data_t DeviceSet::get_data(std::vector<int> port_list) {
    if (stop_requested) {
        logger::log(LEVEL_WARN, "Device set stops measuring");
        stop_requested = false;

        return data_t{};
    }

    logger::log(LEVEL_TRACE, "Preparing to acquire data");
    data_t acquired_data{};

    try {
        if (!traces_configured) {
            vna->create_traces(port_list, using_ext_gen);
            traces_configured = true;
        }

        logger::log(LEVEL_TRACE, "Traces created");
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't create traces");
        return acquired_data;
    }

    for (int port_pos = 0; port_pos < port_list.size(); ++port_pos) {
        if (stop_requested) {
            logger::log(LEVEL_WARN, "Device set stops measuring");
            stop_requested = false;

            return data_t{};
        }

        int port_num = port_list[port_pos];
        logger::log(LEVEL_TRACE, "Port = {}", port_num);

        if (meas_type == MEAS_TRANSITION && port_pos == 0) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_on();
                } else {
                    vna->rf_on(vna->get_source_port());
                }

                logger::log(LEVEL_TRACE, "Source port enabled");
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't enable source port");
                return acquired_data;
            }
        } else if (meas_type == MEAS_REFLECTION) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_on();
                } else {
                    vna->rf_on(port_num);
                }

                logger::log(LEVEL_TRACE, "Port {} enabled", port_num);
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't enable port {}", port_num);
                return acquired_data;
            }
        }

        if ((port_pos == 0 && meas_type == MEAS_TRANSITION) || meas_type == MEAS_REFLECTION) {
            try {
                vna->trigger();
                vna->init();

                logger::log(LEVEL_TRACE, "Measurements restarted");
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't restart measurement");
                return acquired_data;
            }
        }

        try {
            acquired_data.insert_iq_port_data(vna->get_data(port_pos));
            logger::log(LEVEL_TRACE, "Data for port {} acquired", port_num);
        } catch (int error_code) {
            logger::log(LEVEL_ERROR, "Can't acquire data for port {} from VNA", port_num);
            return acquired_data;
        }

        if (meas_type == MEAS_TRANSITION && port_pos == port_list.size() - 1) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_off();
                } else {
                    vna->rf_off(vna->get_source_port());
                }

                logger::log(LEVEL_TRACE, "Source port disabled");
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't disable source port");
                return acquired_data;
            }
        } else if (meas_type == MEAS_REFLECTION) {
            try {
                if (using_ext_gen) {
                    ext_gen->rf_off();
                } else {
                    vna->rf_off(port_num);
                }

                logger::log(LEVEL_TRACE, "Port {} disabled", port_num);
            } catch (int error_code) {
                logger::log(LEVEL_ERROR, "Can't disable port {}", port_num);
                return acquired_data;
            }
        }
    }

    acquired_data.insert_angles(get_current_angles());

    if (using_ext_gen) {
        acquired_data.insert_freq(get_current_freq());
    } else {
        acquired_data.insert_freq_list(get_freq_list());
    }

    logger::log(LEVEL_DEBUG, "Data acquired");

    return acquired_data;
}

void DeviceSet::request_stop() {
    stop_requested = true;
}

void DeviceSet::reset_stop_request() {
    stop_requested = false;
}
