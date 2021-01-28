#pragma once

#include <bits/stdint-intn.h>
#include <cstdio>
#include <memory>
#include <utility>
namespace shmpy {

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
  bool    readonly = false;
  char    format[3];

  // shape
  ssize_t* shape();
  ssize_t* strides();
  void*    ptr();
};

///
/// \brief Memory layout of Python Pickle object
///
struct Py_Pickle
{
  char type_desc[128];

  // data ptr
  char* ptr();
};

} // namespace shmpy
