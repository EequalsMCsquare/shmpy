#include "py_message.hpp"
#include "py_variable.hpp"

#include <cstring>
#include <segment.hpp>

namespace shmpy {
RESP_Failure::RESP_Failure(std::string_view message)
{
  if (message.size() >= 256) {
    // message incomplete copy.
  }
  std::strncpy(this->message, message.data(), 256);
}
RESP_Failure::RESP_Failure(const char* message)
{
  if (std::strlen(message) >= 256) {
    // message incomplete copy
  }
  std::strncpy(this->message, message, 256);
}

RESP_GetVariable::RESP_GetVariable(std::shared_ptr<variable_desc> var_desc)
{
  this->access             = var_desc->access_type;
  this->dtype              = var_desc->dtype;
  this->is_pybuff_protocol = var_desc->is_pybuff_protocol;
}
} // namespace shmpy
