#include "py_except.hpp"
#include <fmt/format.h>
#include <system_error>

namespace shmpy {
ShmpyExcept::ShmpyExcept(ShmpyErrc ec) noexcept
{
  std::error_code __tmp_ec = ec;
  this->_M_What =
    fmt::format("{}({}) {}", __tmp_ec.category().name(), __tmp_ec.value(), __tmp_ec.message());
}

ShmpyExcept::ShmpyExcept(std::error_code ec) {
  this->_M_What = fmt::format("{}({}) {}", ec.category().name(), ec.value(), ec.message());
}

const char* ShmpyExcept::what() const noexcept {
  return this->_M_What.c_str();
}

}