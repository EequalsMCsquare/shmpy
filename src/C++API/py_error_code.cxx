#include "py_error_code.hpp"
#include <system_error>

namespace {
struct ShmpyErrCategory : std::error_category
{
  const char* name() const noexcept override final;
  std::string message(int) const override final;
};

const char*
ShmpyErrCategory::name() const noexcept
{
  return "shmpy_error";
}

std::string
ShmpyErrCategory::message(int ec) const
{
  switch (static_cast<ShmpyErrc>(ec)) {
    case ShmpyErrc::NoError:
      return "no error";
    case ShmpyErrc::NullptrBuffer:
      return "buffer is nullptr";
    case ShmpyErrc::UnableToGetVariableBuffer:
      return "unable to retrieve variable buffer";
    case ShmpyErrc::UnableToGetSegment:
      return "unable to retrieve variable's segment";
    case ShmpyErrc::VariableExist:
      return "variable's name already exist";
    case ShmpyErrc::VariableNotFound:
      return "unable to locate the variable";
    default:
      return "unknown error";
  }
}
const ShmpyErrCategory TheShmpyErrCategory{};
}

std::error_code
make_error_code(ShmpyErrc ec)
{
  return { static_cast<int>(ec), TheShmpyErrCategory };
}