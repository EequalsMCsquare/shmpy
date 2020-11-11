cmake_minimum_required(VERSION 3.14)
project(libshmpy)
set(CMAKE_CXX_STANDARD 17)

find_package(pybind11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

include_directories(pybind11::headers spdlog_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include GTest::gtest)

add_library(shmpy SHARED ${PROJECT_SOURCE_DIR}/src/wrapper.cxx)
target_link_libraries(
    shmpy
    PRIVATE
    pybind11::module
    pybind11::embed
    spdlog::spdlog
    spdlog::spdlog_header_only

    pthread
)

add_executable(shmpy_test ${PROJECT_SOURCE_DIR}/tests/Test_Pool.cxx)
target_link_libraries(
    shmpy_test
    PRIVATE
    pybind11::module
    pybind11::embed
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    spdlog::spdlog_header_only
    pthread
)


pybind11_extension(shmpy)
pybind11_strip(shmpy)