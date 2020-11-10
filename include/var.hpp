#pragma once

#include "libshmpy.hpp"
namespace py = pybind11;

struct VarMeta
{
    char name[64];
    int shmid;
    bool isPyBuffProtocol;
    _DTYPE dtype;
    std::size_t bytes;
    uint32_t ref_count;
    pid_t owner_pid;
    std::mutex mtx;

    // *attaching_pools;
};

class Var
{
friend class PoolBase;

private:
    void *pBuffer;
    int *shmid;
    std::string_view name;
    bool *isPyBuffProtocol;
    _DTYPE *dtype;
    std::size_t *bytes;
    uint32_t *ref_count;
    pid_t *owner_pid;
    std::mutex *mtx;
    // attached_clients

    std::uint32_t *att_pools()
    {
        return (std::uint32_t *)(mtx + 1);
    }
};