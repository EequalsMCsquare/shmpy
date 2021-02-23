#include "py_ec.hpp"

struct ShmpyErrorCategory : std::error_category
{
  const char* name() const noexcept override final { return "shmpy"; }
  std::string message(int ev) const override final
  {
    switch (static_cast<ShmpyErrc>(ev)) {
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
};

const ShmpyErrorCategory TheShmpyErrorCategory{};

std::error_code
make_error_code(ShmpyErrc ec)
{
  return { static_cast<int>(ec), TheShmpyErrorCategory };
}