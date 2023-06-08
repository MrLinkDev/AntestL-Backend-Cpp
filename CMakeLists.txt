cmake_minimum_required(VERSION 3.25)
project(antestl_backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(libs/ktvisa libs/nlohman)
link_directories(libs)

add_executable(
        antestl_backend

        src/main.cpp

        src/socket/server.hpp

        src/utils/logger.hpp
        src/utils/visa_device.hpp
        src/utils/device_set.hpp
        src/utils/task_manager.hpp
        src/utils/utils.hpp

        src/devices/ext_gen/ext_gen_device.hpp

        src/devices/rbd/rbd_device.hpp
        src/devices/rbd/rbd_2.cpp

        src/devices/vna/vna_device.hpp
        src/devices/vna/keysight_m9807a.cpp)

target_link_libraries(
        antestl_backend

        ktvisa32.lib
        ktvisaext.lib
        visa32.lib
        visaext.lib

        wsock32
        ws2_32)