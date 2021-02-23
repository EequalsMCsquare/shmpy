#include "py_except.hpp"
#include "py_server.hpp"

namespace shmpy {

void
Py_Server::Py_InsertInt32(std::string_view name, const int32_t number)
{
  std::error_code ec;
  this->HANDLE_CacheInsert<DTYPE::int32>(
    name,
    sizeof(int32_t),
    this->id(),
    [&number](void* buffer) { *(static_cast<int32_t*>(buffer)) = number; },
    ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}
}