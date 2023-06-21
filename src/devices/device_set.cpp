#include "device_set.hpp"

bool DeviceSet::connect(int device_type, std::string device_model, std::string device_address) {
    std::transform(device_model.begin(), device_model.end(), device_model.begin(), ::toupper);

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
            try {
                if (device_model == "tesart") {
                    rbd = new TesartRbd(device_address);
                } else if (device_model == "upkb") {
                    //TODO: Добавить инициализацию объекта
                } else {
                    return false;
                }
            } catch (int error_code) {
                return false;
            }

        default:
            return false;
    }
}
