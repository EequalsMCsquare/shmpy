#include "py_variable.hpp"

namespace shmpy {

variable_desc::variable_desc(
  const shm_kernel::message_handler::msg_head*              msg_head,
  const REQ_InsertVariable*                                 req,
  std::shared_ptr<shm_kernel::memory_manager::base_segment> segment)
{
  this->dtype              = req->dtype;
  this->size               = req->size;
  this->is_pybuff_protocol = req->is_pybuff_protocol;
  this->segment            = segment;
  this->ref_count          = 1;
  this->attach_ids.push_back(msg_head->from);
}
} // namespace shmpy
