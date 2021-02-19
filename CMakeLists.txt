cmake_minimum_required(VERSION 3.15)
project(shmpy
        VERSION 0.7.5)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

option(BUILD_TESTING "" ON)

find_package(ipc REQUIRED)
find_package(message_handler REQUIRED)
find_package(memory_manager REQUIRED)
find_package(cppzmq REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Catch2)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/C++API API_SRC)

add_library(shmpy_C++API STATIC ${API_SRC})
target_include_directories(shmpy_C++API PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(shmpy_C++API PUBLIC cxx_std_17)
target_compile_options(shmpy_C++API PUBLIC -fPIC)
target_link_libraries(shmpy_C++API PUBLIC 
    shm_kernel::ipc
    shm_kernel::memory_manager
    shm_kernel::message_handler
    pybind11::module 
    pybind11::embed
    spdlog::spdlog_header_only
    pthread
)

if (BUILD_TESTING)
    add_subdirectory(./tests)
endif()

add_library(shmpy SHARED "")
target_sources(shmpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/shmpy_impl.cxx)
add_custom_command(TARGET shmpy
    POST_BUILD
    COMMAND stubgen -m shmpy -o .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/module
    COMMENT "generating function signature")

add_custom_command(TARGET shmpy
    POST_BUILD
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/tests/py/*.py build/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "copy python testcase")

set_target_properties(shmpy
    PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/module/"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/module/"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/module/"
    )
target_include_directories(shmpy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    shmpy
    shmpy_C++API
)
pybind11_extension(shmpy)
pybind11_strip(shmpy)
