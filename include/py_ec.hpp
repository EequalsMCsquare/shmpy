#pragma once

#include "py_common.hpp"

enum class ShmpyErrc
{
  NoError = 0,
  UnableToGetSegment,
  UnableToGetVariableBuffer,
  NullptrBuffer,
  VariableExist,
  VariableNotFound,
};

namespace std {
template<>
struct is_error_code_enum<ShmpyErrc> : true_type
{};
}

std::error_code make_error_code(ShmpyErrc);
