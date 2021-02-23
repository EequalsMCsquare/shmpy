#include "py_variable.hpp"
#include <mutex>

namespace shmpy {
variable_desc::variable_desc(const DTYPE dtype, const size_t size, const bool is_bp)
  : dtype_(dtype)
  , is_bp_(is_bp)
  , size_(size)
  , ref_count_(0)
{}

variable_desc::variable_desc(const DTYPE          dtype,
                             const size_t         size,
                             const bool           is_bp,
                             sptr<base_segment_t> seg)
  : variable_desc(dtype, is_bp, size)
{
  this->segment_ = seg;
}

void
variable_desc::make_client_attached(const uint32_t client_id) noexcept
{
  std::lock_guard __lock(this->mtx_);
  auto            __iter = std::find(this->attach_ids_.begin(), this->attach_ids_.end(), client_id);
  if (__iter == this->attach_ids_.end()) {
    this->attach_ids_.emplace_back(client_id);
    this->ref_count_++;
  }
}

segment_info_t
variable_desc::get_segment_info() const noexcept
{
  return std::move(this->segment_->to_seginfo());
}
}