cmake_minimum_required(VERSION 3.14)

find_package(Catch2 REQUIRED)
add_executable(Test_PyServer ./cxx/Test_PyServer.cxx)
target_link_libraries(Test_PyServer PRIVATE shmpy_C++API Catch2::Catch2)

find_package(Python3 REQUIRED)
add_test(NAME 
            Python_Testcase
        COMMAND 
            ${PYTHON_EXECUTABLE} ./testcase_server.py
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}/build
)
set_tests_properties(Python_Testcase 
  PROPERTIES 
    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/build:$ENV{PYTHONPATH}")