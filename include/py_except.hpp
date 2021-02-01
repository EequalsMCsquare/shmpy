#pragma once

#include "py_error_code.hpp"
#include <exception>
#include <stdexcept>
namespace shmpy {

class ShmpyExcept : std::exception
{

private:
  std::string _M_What;

public:
  ShmpyExcept(ShmpyErrc);
  ShmpyExcept(std::error_code);
  const char* what() const noexcept override final;
};

}