#pragma once 

#include "libshmpy.hpp"
namespace py = pybind11;


struct VarMeta {
    char name[64];
    int shmid;
    bool isPyBuffProtocol;
    _DTYPE dtype;
    std::size_t bytes;
    uint32_t ref_count;
    pid_t owner_pid;
    std::mutex mtx;

    std::uint32_t* attaching_clients;
};

class Var {

private:
    void* pBuffer;
    int* shmid;
    std::string_view name;
    bool* isPyBuffProtocol;
    _DTYPE* dtype;
    std::size_t* bytes;
    uint32_t* ref_count;
    pid_t* owner_pid;
    std::mutex* mtx;

    std::uint32_t* attaching_clients() {
        return (std::uint32_t*)(mtx + 1);
    }
};