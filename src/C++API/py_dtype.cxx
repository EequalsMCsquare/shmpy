#include "py_dtype.hpp"

namespace shmpy {

std::function<void(void*)>
Py_Int32::MemcpyFunc(const int32_t data)
{
  return [&data](void* buffer) { *(static_cast<int32_t*>(buffer)) = data; };
}

int32_t
Py_Int32::Rebuild(const void* buffer)
{
  int32_t __tmp;
  std::memcpy(&__tmp, buffer, sizeof(int32_t));
  return __tmp;
}
}