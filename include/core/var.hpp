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
    shmpy::_DTYPE dtype;
    std::size_t nbytes;
    uint32_t ref_count;
    std::mutex mtx;

    // *att_pools;
};

class Var {
    friend class PoolBase;

public:
    Var() = default;
    Var(VarMeta* _meta, void* _pBuffer) {
        // This is only used to build Var from freshly attached VarMeta
        if (_meta == nullptr)
            throw var_error("var meta is nullptr.");
        this->name = _meta->name;
        this->shmid = &_meta->shmid;
        this->isPyBuffProtocol = &_meta->isPyBuffProtocol;
        this->dtype = &_meta->dtype;
        this->nbytes = &_meta->nbytes;
        this->ref_count = &_meta->ref_count;
        this->mtx = &_meta->mtx;
        this->pBuffer = _pBuffer;
    }

    const std::string_view get_name()
    {
        return this->name;
    }

    const shmpy::_DTYPE get_dtype()
    {
        return *this->dtype;
    }

private:
    void* pBuffer; // when variable actual data attached, pBuffer will automatically be assigned.

    std::string_view name;
    int* shmid;
    bool* isPyBuffProtocol;
    shmpy::_DTYPE* dtype;
    std::size_t* nbytes;
    uint32_t* ref_count;
    std::mutex* mtx;

    // att_pools
    std::uint32_t* att_pools()
    {
        return (std::uint32_t*)(this->mtx+1);
    }
};