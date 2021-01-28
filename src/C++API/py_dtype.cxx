#include "py_dtype.hpp"

namespace shmpy {
ssize_t*
Py_BufferProtocol::shape()
{
  return reinterpret_cast<ssize_t*>(this->format + 3);
  // strides
}
ssize_t*
Py_BufferProtocol::strides()
{
  return reinterpret_cast<ssize_t*>(this->shape() + this->ndims);
}

// data ptr
void*
Py_BufferProtocol::ptr()
{
  return reinterpret_cast<void*>(this->strides() + this->ndims);
}

char*
Py_Pickle::ptr()
{
  return this->type_desc + 128;
}
} // namespace shmpy