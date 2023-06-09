cmake_minimum_required(VERSION 3.25)
project(antestl_backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(libs/ktvisa libs/nlohman)
link_directories(libs)

add_executable(
        antestl_backend

        src/main.cpp

        src/utils/utils.hpp
        src/utils/logger.hpp

        src/utils/socket/socket_server.hpp
        src/utils/socket/socket_server.cpp)

target_link_libraries(
        antestl_backend

        ktvisa32.lib
        ktvisaext.lib
        visa32.lib
        visaext.lib

        wsock32
        ws2_32)