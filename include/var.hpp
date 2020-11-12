#pragma once

#include "libshmpy.hpp"
namespace py = pybind11;

enum var_status {
    VAR_OK,
    VAR_DT,
    VAR_AB,
};

struct VarMeta {
    char name[64];
    int shmid;
    var_status status;
    bool isPyBuffProtocol;
    _DTYPE dtype;
    std::size_t nbytes;
    uint32_t ref_count;
    std::mutex mtx;

    // *att_pools;
};

class Var {
    friend class PoolBase;

public:
    const std::string_view get_name()
    {
        return this->name;
    }

    const _DTYPE get_dtype()
    {
        return *this->dtype;
    }

private:
    void* pBuffer;

    std::string_view name;
    int* shmid;
    bool* isPyBuffProtocol;
    _DTYPE* dtype;
    std::size_t* nbytes;
    uint32_t* ref_count;
    std::mutex* mtx;

    // att_pools
    std::uint32_t* att_pools()
    {
        return (std::uint32_t*)(&static_cast<VarMeta*>(pBuffer)->mtx + 1);
    }
};