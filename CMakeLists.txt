cmake_minimum_required(VERSION 3.15)
project(shmpy
        VERSION 0.7.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(shared_memory REQUIRED)
find_package(message_handler REQUIRED)
find_package(memory_manager REQUIRED)
find_package(cppzmq REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Catch2)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/C++API API_SRC)

add_library(shmpy_C++API STATIC ${API_SRC})
target_include_directories(shmpy_C++API PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(shmpy_C++API PRIVATE -fPIC)
target_link_libraries(shmpy_C++API PUBLIC 
    shm_kernel::shared_memory
    shm_kernel::memory_manager
    shm_kernel::message_handler
    pybind11::module 
    pybind11::embed
    spdlog::spdlog_header_only
    pthread
)

option(BUILD_TEST "" ON)
if (BUILD_TEST)
    add_subdirectory(./tests)
endif()

add_library(shmpy SHARED src/shmpy_impl.cxx)
target_include_directories(shmpy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    shmpy
    shmpy_C++API
)
pybind11_extension(shmpy)
pybind11_strip(shmpy)
