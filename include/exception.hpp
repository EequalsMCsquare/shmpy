#pragma once 

#include <stdexcept>

class shm_error : public std::runtime_error {
public:
    shm_error(const std::string& msg) : runtime_error(msg) {}
    shm_error(int err): runtime_error(strerror(err)){}
};

class msgq_error :public std::runtime_error{
public:
    msgq_error(const std::string& msg) : runtime_error(msg) {}
};

class pool_error : public std::runtime_error {
public:
    pool_error(const std::string& msg) : runtime_error(msg) {}
};

class var_error : public std::runtime_error {
public:
    var_error(const std::string& msg) : runtime_error(msg) {}
};
