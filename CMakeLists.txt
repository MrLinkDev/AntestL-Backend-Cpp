cmake_minimum_required(VERSION 3.25)
project(antestl_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(libs/ktvisa)
link_directories(libs)

add_executable(
        antestl_backend
        src/main.cpp
        src/socket/server.hpp
        src/utils/logger.hpp
        src/utils/visa_device.hpp)

target_link_libraries(
        antestl_backend
        ktvisa32.lib
        ktvisaext.lib
        visa32.lib
        visaext.lib
        wsock32
        ws2_32)