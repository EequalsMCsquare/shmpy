#pragma once

#include "py_common.hpp"
#include <list>

namespace shmpy {

enum class DTYPE;

/**
 * @brief variable status
 *
 */
enum class VAR_STATUS
{
  READY,
  NOT_READY,
  MARK_DELETE,
};

class variable_desc
{
private:
  DTYPE                dtype_;
  bool                 is_bp_;
  size_t               size_;

  std::string_view     name_;
  std::atomic_uint32_t ref_count_;
  std::list<uint32_t>  attach_ids_;
  std::mutex           mtx_;
  sptr<base_segment_t> segment_;

  variable_desc(const DTYPE dtype, const bool is_bp, const size_t size);

public:
  variable_desc(const DTYPE dtype, const bool is_bp, const size_t size, sptr<base_segment_t>);

  /**
   * @brief if a client id is not in attach_ids, it will append it, otherwise do
   * nothing.
   *
   * @param client_id
   */
  void make_client_attached(const uint32_t client_ids) noexcept;
};

class attached_variable
{
private:
  DTYPE dtype_;
  bool is_bp_;
  size_t size_;

  std::atomic_size_t local_ref_count_;

};

}