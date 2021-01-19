#pragma once

#include <bits/stdint-intn.h>
#include <cstdio>
#include <memory>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <utility>
namespace shmpy {

namespace py = pybind11;

enum class DTYPE
{
  INT,
  FLOAT,
  BOOL,
  CHAR,
  PY_LIST,
  PY_DICT,
  PY_STRING,

  PY_BUFF_PROTOCOL,
  PY_PICKLE,
};

///
/// \brief Memory layout of Python Buffer Protocol object
///
struct Py_BufferProtocol
{
  ssize_t itemsize;
  ssize_t size;
  ssize_t ndims;
  bool readonly = false;
  char format[2];

  // shape
  ssize_t* shape()
  {
    //
    return reinterpret_cast<ssize_t*>(this->format + 7);
  }

  // strides
  ssize_t* strides()
  {
    return reinterpret_cast<ssize_t*>(this->shape() + this->ndims);
  }

  // data ptr
  void* ptr() { return reinterpret_cast<void*>(this->strides() + this->ndims); }
};

///
/// \brief Memory layout of Python Pickle object
///
struct Py_Pickle
{
  char type_desc[128];

  // data ptr
  void* ptr() { return reinterpret_cast<void*>(this->type_desc + 128); }
};

} // namespace shmpy
