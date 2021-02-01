#pragma once
#include <system_error>
#include <type_traits>

enum class ShmpyErrc
{
  NoError,
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