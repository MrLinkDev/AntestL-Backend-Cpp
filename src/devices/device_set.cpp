#include "device_set.hpp"
#include "rbd/demo_rdb.hpp"

bool DeviceSet::connect(int device_type, std::string device_model, std::string device_address) {
    std::transform(device_model.begin(), device_model.end(), device_model.begin(), ::toupper);

    try {
        switch (device_type) {
            case DEVICE_VNA:
                if (device_model == "M9807A") {
                    vna = new KeysightM9807A(device_address);
                    vna->preset();
                } else {
                    return false;
                }

                return vna->is_connected();
            case DEVICE_GEN:
                ext_gen = new KeysightGen(device_address);

                return ext_gen->is_connected();
            case DEVICE_RBD:
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
    } catch (int error_code) {
        return false;
    }
}

bool DeviceSet::configure(int meas_type, float rbw, int source_port, bool using_ext_gen) {
    try {
        vna->full_preset();
        logger::log(LEVEL_DEBUG, "Made full preset");

        vna->init_channel();
        logger::log(LEVEL_DEBUG, "Default channel initialized");

        vna->configure(meas_type, rbw, source_port, using_ext_gen);

        this->meas_type = meas_type;
        this->using_ext_gen = using_ext_gen;
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't configure VNA");
        return false;
    }

    logger::log(LEVEL_DEBUG, "VNA configured");
    return true;
}

bool DeviceSet::set_power(float power) {
    try {
        if (using_ext_gen) {
            ext_gen->set_power(power);
            logger::log(LEVEL_DEBUG, "External gen power = {}", power);
        } else {
            vna->set_power(power);
            logger::log(LEVEL_DEBUG, "VNA power = {}", power);
        }
    } catch (int error_code) {
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
    } catch (int error_code) {
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
    } catch (int error_code) {
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
        ext_gen->next_freq();
    } catch (int error_code) {
        if (error_code == FREQ_OUT_OF_BOUND) {
            logger::log(LEVEL_WARN, "Trying to set frequency out of range!");
            return NEXT_FREQ_BOUND;
        } else {
            logger::log(LEVEL_ERROR, "Can't set frequency on external gen");
            return NEXT_FREQ_ERROR;
        }
    }

    return NEXT_FREQ_OK;
}

int DeviceSet::prev_freq() {
    try {
        ext_gen->next_freq();
    } catch (int error_code) {
        if (error_code == FREQ_OUT_OF_BOUND) {
            logger::log(LEVEL_WARN, "Trying to set frequency out of range!");
            return NEXT_FREQ_BOUND;
        } else {
            logger::log(LEVEL_ERROR, "Can't set frequency on external gen");
            return NEXT_FREQ_ERROR;
        }
    }

    return NEXT_FREQ_OK;
}

bool DeviceSet::move_to_start_freq() {
    if (using_ext_gen) {
        try {
            ext_gen->move_to_start_freq();
        } catch (int error_code) {
            logger::log(LEVEL_ERROR, "Can't set frequency on external gen");
            return false;
        }
    }

    return true;
}

bool DeviceSet::set_angle(float angle, int axis_num) {
    try {
        rbd->set_angle(angle, axis_num);
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't set angle on RBD");
        return false;
    }

    logger::log(LEVEL_INFO, "RBD (axis {}): angle = {}", axis_num, angle);
    return true;
}

bool DeviceSet::set_angle_range(float start_angle, float stop_angle, int points, int axis_num) {
    try {
        rbd->set_angle_range(start_angle, stop_angle, points, axis_num);
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't set angle range on RBD");
        return false;
    }

    logger::log(LEVEL_INFO, "RBD (axis {}): angle range = [{}; {}] ({} points)", axis_num, start_angle, stop_angle, points);
    return true;
}

int DeviceSet::next_angle(int axis_num) {
    try {
        rbd->next_angle(axis_num);
    } catch (int error_code) {
        if (error_code == ANGLE_OUT_OF_BOUND) {
            //logger::log(LEVEL_ERROR, "Trying to set angle out of range (axis {})!", axis_num);
            return NEXT_ANGLE_BOUND;
        } else {
            logger::log(LEVEL_ERROR, "Can't set angle on RBD (axis {})", axis_num);
            return NEXT_ANGLE_ERROR;
        }
    }

    return NEXT_ANGLE_OK;
}

int DeviceSet::prev_angle(int axis_num) {
    try {
        rbd->prev_angle(axis_num);
    } catch (int error_code) {
        if (error_code == ANGLE_OUT_OF_BOUND) {
            //logger::log(LEVEL_ERROR, "Trying to set angle out of range (axis {})!", axis_num);
            return NEXT_ANGLE_BOUND;
        } else {
            logger::log(LEVEL_ERROR, "Can't set angle on RBD (axis {})", axis_num);
            return NEXT_ANGLE_ERROR;
        }
    }

    return NEXT_ANGLE_OK;
}

bool DeviceSet::move_to_start_angle(int axis_num) {
    try {
        rbd->move_to_start_angle(axis_num);
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't set angle on RBD (axis {})", axis_num);
        return false;
    }

    return true;
}

bool DeviceSet::change_path(std::vector<int> path_list) {
    try {
        vna->set_path(path_list);
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't change switch paths on VNA");
        return false;
    }

    logger::log(LEVEL_INFO, "Changed switch paths on VNA");
    return true;
}

std::vector<iq_data_t> DeviceSet::get_data(std::vector<int> port_list) {
    logger::log(LEVEL_TRACE, "Preparing to acquire data");

    std::vector<iq_data_t> acquired_data{};

    try {
        vna->create_traces(port_list, using_ext_gen);
        logger::log(LEVEL_TRACE, "Traces created");
    } catch (int error_code) {
        logger::log(LEVEL_ERROR, "Can't create traces");
        return acquired_data;
    }

    for (int port_pos = 0; port_pos < port_list.size(); ++port_pos) {
        int port_num = port_list[port_pos];
        logger::log(LEVEL_TRACE, "Port = {}", port_num);

        if (meas_type == MEAS_TRANSITION) {
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

        try {
            vna->trigger();
            vna->init();
            logger::log(LEVEL_TRACE, "Measurements restarted");
        } catch (int error_code) {
            logger::log(LEVEL_ERROR, "Can't restart measurement");
            return acquired_data;
        }

        try {
            acquired_data.push_back(vna->get_data(port_pos));
            logger::log(LEVEL_TRACE, "Data for port {} acquired", port_num);
        } catch (int error_code) {
            logger::log(LEVEL_ERROR, "Can't acquire data for port {} from VNA", port_num);
            return acquired_data;
        }

        if (meas_type == MEAS_TRANSITION) {
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

    return acquired_data;
}

int DeviceSet::get_vna_switch_module_count()  {
    if (vna != nullptr && vna->is_connected()) {
        return vna->get_switch_module_count();
    } else {
        return 0;
    }
}

std::string DeviceSet::get_current_angle_list() {
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

double DeviceSet::get_current_freq(int point_pos) {
    if (using_ext_gen) {
        if (ext_gen != nullptr && ext_gen->is_connected()) {
            return ext_gen->get_current_freq();
        } else {
            return 0.0;
        }
    } else {
        return vna->get_freq_by_point_num(point_pos);
    }
}

bool DeviceSet::is_using_ext_gen() {
    return using_ext_gen;
}

