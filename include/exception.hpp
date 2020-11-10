#pragma once 

#include <stdexcept>

class shm_error : std::runtime_error {
public:
    shm_error(const std::string& msg) : runtime_error(msg) {}
};

class msg_error :std::runtime_error {
public:
    msg_error(const std::string& msg) : runtime_error(msg) {}
};

class pool_error : std::runtime_error {
public:
    pool_error(const std::string& msg) : runtime_error(msg) {}
};