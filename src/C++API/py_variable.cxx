#include "py_variable.hpp"
#include <algorithm>
#include <mutex>

namespace shmpy {

variable_desc::variable_desc(const DTYPE                                               dtype,
                             const size_t                                              size,
                             const bool                                                is_bp,
                             const uint32_t                                            from_id,
                             std::shared_ptr<shm_kernel::memory_manager::base_segment> segment)
{
  this->dtype              = dtype;
  this->size               = size;
  this->is_bp              = is_bp;
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
attached_variable::attached_variable(std::shared_ptr<libmem::segmentdesc> seg,
                                     std::shared_ptr<shm_kernel::shared_memory::shm_handle> shm,
                                     const variable_desc* var,
                                     const size_t meta_pshift,
                                     const size_t buffer_pshift)
{
  this->segment_descriptor = seg;
  this->shm_handle = shm;
  char* __buff_head = static_cast<char*>(shm->map());
  this->variable_meta = __buff_head + meta_pshift;
  this->variable_buffer = __buff_head + buffer_pshift;
  this->dtype = var->dtype;
  this->access_type = var->access_type;
  this->is_bp              = var->is_bp;
  this->size = var->size;
  this->local_ref_count = 1;
}
libmem::segmentdesc variable_desc::get_segmentdesc() const noexcept {
 return this->segment->to_segmentdesc();
}

} // namespace shmpy
