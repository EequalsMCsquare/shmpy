#pragma once
#include "py_common.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <pybind11/pytypes.h>
#include <type_traits>

namespace shmpy {

namespace py = pybind11;

enum class DTYPE
{
  int32   = 0,
  int64   = 1,
  uint32  = 2,
  uint64  = 3,
  float32 = 4,
  float64 = 5,
  boolean = 6,

  py_list   = 20,
  py_dict   = 21,
  py_string = 22,
};

template<typename T, DTYPE __dtype>
struct Py_Dtype
{
  using type                   = T;
  static constexpr DTYPE dtype = __dtype;
};

struct Py_Int32 : Py_Dtype<int32_t, DTYPE::int32>
{
  using PyType = py::int_;
  static std::function<void(void*)> MemcpyFunc(const int32_t data);
  static int32_t                    Rebuild(const void* buffer);
};

struct Py_Int64 : Py_Dtype<int64_t, DTYPE::int64>
{
  using PyType = py::int_;
  static std::function<void(void*)> MemcpyFunc(const int64_t data);
  static int64_t Rebuild(const void* buffer);
};

using Py_UInt32  = Py_Dtype<uint32_t, DTYPE::uint32>;
using Py_UInt64  = Py_Dtype<uint64_t, DTYPE::uint64>;
using Py_Float32 = Py_Dtype<float, DTYPE::float32>;
using Py_Float64 = Py_Dtype<double, DTYPE::float64>;
using Py_Bollean = Py_Dtype<bool, DTYPE::boolean>;

struct Py_String : Py_Dtype<std::string_view, DTYPE::py_string>
{
  size_t      size_;
  const char* data() const;

  static std::function<void(void*)> MemcpyFunc(std::string_view src_data);
  static std::string_view Rebuild(const void* buffer);
};
}