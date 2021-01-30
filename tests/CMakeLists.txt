cmake_minimum_required(VERSION 3.14)

find_package(Catch2 REQUIRED)
add_executable(Test_PyServer ./cxx/Test_PyServer.cxx)
target_link_libraries(Test_PyServer PRIVATE shmpy_C++API Catch2::Catch2)

