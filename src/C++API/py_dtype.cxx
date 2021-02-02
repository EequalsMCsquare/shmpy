#include "py_dtype.hpp"

namespace shmpy {
const char*
Py_String::data() const
{
  return reinterpret_cast<const char*>(&size_ + 1);
}

} // namespace shmpy