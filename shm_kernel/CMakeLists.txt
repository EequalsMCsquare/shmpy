cmake_minimum_required(VERSION 3.14)

option(BUILD_SHM_TEST OFF)
option(BUILD_MEM_TEST OFF)
option(BUILD_MSG_TEST OFF)

add_subdirectory(./shared_memory)
add_subdirectory(./memory_manager)
add_subdirectory(./message_handler)
