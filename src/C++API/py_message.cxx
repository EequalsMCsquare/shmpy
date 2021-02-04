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
template<int msg_type>
RESP_VariableShmInsert<msg_type>::RESP_VariableShmInsert(std::shared_ptr<variable_desc> var_desc)
{
  this->success = true;
  this->segment = var_desc->get_segmentdesc();
  this->access_type = var_desc->access_type;
  this->dtype = var_desc->dtype;
  this->is_bp = var_desc->is_bp;
  this->size = var_desc->size;
}
template<int msg_type>
RESP_VariableShmInsert<msg_type>::RESP_VariableShmInsert(std::string_view __error)
{
  this->success = false;
  std::strncpy(this->msg, __error.data(), 128);
}
template<int msg_type>
RESP_VariableShmInsert<msg_type>::RESP_VariableShmInsert(
  std::shared_ptr<libmem::base_segment> segment,
  const DTYPE                           dtype,
  const ACCESS_TYPE                     access_type,
  const bool                            is_bp,
  const size_t                          size)
{
  this->success = true;
  this->segment = segment->to_segmentdesc();
  this->dtype = dtype;
  this->access_type = access_type;
  this->is_bp = is_bp;
  this->size = size;
}
} // namespace shmpy
