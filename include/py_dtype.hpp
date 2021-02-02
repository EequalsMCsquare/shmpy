#pragma once

#include <bits/stdint-intn.h>
#include <bits/types/FILE.h>
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
  ssize_t itemsize_;
  ssize_t size_;
  ssize_t ndims_;
  bool    readonly_ = false;
  char    format_[3];

  const ssize_t* shape() const;
  const ssize_t* strides() const;
  const void*    data() const;
};

struct Py_String
{
  size_t      size_;
  const char* data() const;
};

} // namespace shmpy
