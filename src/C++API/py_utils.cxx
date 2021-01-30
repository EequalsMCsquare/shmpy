#include "py_utils.hpp"
#include <cstring>

namespace shmpy::utils {

long
LONG_FromPtr(void* const ptr)
{
  return *(reinterpret_cast<long*>(ptr));
}
}