cmake_minimum_required(VERSION 3.14)

add_library(Testcase_mainobj OBJECT cxx/Testcase_main.cxx)
target_compile_features(Testcase_mainobj PUBLIC cxx_std_17)

enable_testing()
find_package(Catch2 REQUIRED)
add_executable(Testcase_Pool cxx/Testcase_Pool.cxx)
target_link_libraries(Testcase_Pool PRIVATE Testcase_mainobj shmpy_C++API Catch2::Catch2)
add_test(
        NAME
            Cpp_Testcase
        COMMAND
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Testcase_Pool
)

#find_package(Python3 REQUIRED)
#add_test(NAME
#            Python_Testcase
#        COMMAND
#            ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/py/testcase_server.py
#        WORKING_DIRECTORY
#            ${CMAKE_SOURCE_DIR}/build
#)
#set_tests_properties(Python_Testcase
#  PROPERTIES
#    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/build:$ENV{PYTHONPATH}")