#include "py_variable.hpp"
#include <algorithm>
#include <mutex>

namespace shmpy {

variable_desc::variable_desc(const shm_kernel::message_handler::msg_head*              msg_head,
                             const REQ_InsertVariable*                                 req,
                             std::shared_ptr<shm_kernel::memory_manager::base_segment> segment)
{
  this->dtype              = req->dtype;
  this->size               = req->size;
  this->is_pybuff_protocol = req->is_pybuff_protocol;
  this->segment            = segment;
  this->ref_count          = 1;
  this->attach_ids.emplace_back(msg_head->from);
}
variable_desc::variable_desc(const DTYPE                                               dtype,
                             const size_t                                              size,
                             const bool                                                is_bp,
                             const uint32_t                                            from_id,
                             std::shared_ptr<shm_kernel::memory_manager::base_segment> segment)
{
  this->dtype              = dtype;
  this->size               = size;
  this->is_pybuff_protocol = is_bp;
  this->ref_count          = 1;
  this->attach_ids.emplace_back(from_id);
  this->segment = segment;
}
void
variable_desc::add_attached_client(const uint32_t client_id) noexcept
{
  std::lock_guard<std::mutex> __lock(this->mtx);
  auto __iter = std::find(this->attach_ids.begin(), this->attach_ids.end(), client_id);
  if (__iter == this->attach_ids.end()) {
    this->attach_ids.emplace_back(client_id);
    this->ref_count++;
  }
}
} // namespace shmpy
