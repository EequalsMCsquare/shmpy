cmake_minimum_required(VERSION 3.14)
project(libshmpy)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Catch2)

add_subdirectory(shm_kernel)

add_library(shmpy src/shmpy_impl.cxx)
target_include_directories(shmpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    shmpy
    PRIVATE
    shm_kernel::shared_memory
    shm_kernel::message_handler
    shm_kernel::memory_manager
    pybind11::module
    pybind11::embed
    spdlog::spdlog
    pthread
)
pybind11_extension(shmpy)
pybind11_strip(shmpy)
