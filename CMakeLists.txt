cmake_minimum_required(VERSION 3.25)
project(antestl_backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(libs/ktvisa libs/nlohman)
link_directories(libs)

add_executable(
        antestl_backend

        src/main.cpp

        src/task_manager.hpp
        src/task_manager.cpp

        src/utils/utils.hpp
        src/utils/logger.hpp
        src/utils/exceptions.hpp

        src/socket/socket_server.hpp
        src/socket/socket_server.cpp

        src/devices/visa_device.hpp
        src/devices/visa_device.cpp

        src/devices/device_set.hpp
        src/devices/device_set.cpp

        src/devices/vna/vna_device.hpp

        src/devices/vna/keysight_m9807a.hpp
        src/devices/vna/keysight_m9807a.cpp

        src/devices/gen/gen_device.hpp

        src/devices/gen/keysight_gen.hpp
        src/devices/gen/keysight_gen.cpp

        src/devices/rbd/rbd_device.hpp
        src/devices/rbd/demo_rdb.hpp

        src/devices/rbd/tesart_rbd.hpp
        src/devices/rbd/tesart_rbd.cpp
)

target_link_libraries(
        antestl_backend

        ktvisa32.lib
        ktvisaext.lib
        visa32.lib
        visaext.lib

        wsock32
        ws2_32
)