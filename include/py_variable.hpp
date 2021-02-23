#pragma once

#include "py_common.hpp"
#include "py_message.hpp"
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

  friend class RESP_VariableShmInsert;
  friend class Py_Server;

#ifndef NDEBUG
public:
#else
private:
#endif
  DTYPE  dtype_;
  size_t size_;
  bool   is_bp_;

  std::string_view     name_;
  std::atomic_uint32_t ref_count_;
  std::list<uint32_t>  attach_ids_;
  std::mutex           mtx_;
  sptr<base_segment_t> segment_;

  variable_desc(const DTYPE dtype, const size_t size, const bool is_bp);

public:
  variable_desc(const DTYPE dtype, const size_t size, const bool is_bp, sptr<base_segment_t>);

  /**DTYPE
   * @brief if a client id is not in attach_ids, it will append it, otherwise do
   * nothing.
   *
   * @param client_id
   */
  void make_client_attached(const uint32_t client_id) noexcept;

  segment_info_t get_segment_info() const noexcept;
};

}