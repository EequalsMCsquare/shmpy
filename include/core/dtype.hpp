#pragma once

#include "libshmpy.hpp"
#include <map>
#include "mem_literal.hpp"

// auto type_map = std::make_shared<std::map<std::string, py::type>>();

namespace shmpy {

    enum class _DTYPE {
        _INTEGER = 1,
        _FLOAT = 2,
        _BOOL = 3,
        _CHAR = 4,
        _PY_LIST = 10,
        _PY_DICT = 11,
        _PY_OBJ = -1,
        _ND_ARRAY = 20,
    };


    struct buff_protocol {
        ssize_t itemsize;
        ssize_t size;
        char format[4]; // it should be char[2], but the compiler will try to align it. Just set it to 4 bytes at the beginning
        bool readonly = false;
        ssize_t ndims;

        // shape
        ssize_t* shape() {
            return &this->ndims+1;
        }

        // strides
        ssize_t* strides() {
            return this->shape()+this->ndims;
        }

        // data pointer
        void *ptr() {
            return static_cast<void *>(this->strides() + this->ndims);
        }
    };
}
