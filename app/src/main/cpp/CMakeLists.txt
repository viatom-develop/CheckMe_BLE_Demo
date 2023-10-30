
cmake_minimum_required(VERSION 3.18.1)


project("ecg_respiration")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ecg_respiration)
file(GLOB_RECURSE ecg_src
        ${CMAKE_CURRENT_SOURCE_DIR}/ecg_respiration/*.cpp)

add_library(
        ecg_respiration


        SHARED

       ${ecg_src}
        native-lib.cpp)


find_library(
        log-lib


        log)


target_link_libraries(
        ecg_respiration


        ${log-lib})