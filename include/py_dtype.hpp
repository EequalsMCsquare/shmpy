#pragma once

#include <bits/stdint-intn.h>
#include <cstdio>
#include <memory>
#include <string_view>
#include <utility>
namespace shmpy {

enum class DTYPE
{
  INT,
  FLOAT,
  BOOL,
  PY_LIST,
  PY_DICT,

  PY_BUFF_PROTOCOL,
  PY_PICKLE,
  PY_STRING,
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

struct Py_String
{
  size_t size;

  const char* ptr() const;
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
