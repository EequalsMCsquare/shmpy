#pragma once

#include "py_ec.hpp"
#include <stdexcept>

namespace shmpy {
class ShmpyExcept : std::exception
{
private:
  std::string _M_What;

public:
  ShmpyExcept(ShmpyErrc) noexcept;
  ShmpyExcept(std::error_code);
  const char* what() const noexcept override final;
};
}