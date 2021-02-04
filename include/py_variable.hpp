#pragma once

#include "msg/base_msg.hpp"
#include "py_dtype.hpp"
#include "py_message.hpp"
#include "shared_memory.hpp"

#include <segment.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>
#include <string_view>

namespace shmpy {

enum class VAR_STATUS
{
  READY,       //
  NOT_READY,   //
  MARK_DELETE, //
};

enum class ACCESS_TYPE
{
  BY_COPY,      // 通过拷贝
  BY_REF,       // 通过引用
  BY_CONST_REF, // 通过ReadOnly 引用
};

struct base_variable
{
  DTYPE       dtype;
  ACCESS_TYPE access_type;
  bool        is_bp;
  size_t      size;
};

/**
 * \brief 只保存在服务器端
 *
 */
class variable_desc : public base_variable
{

  friend class Py_Server;
  friend struct RESP_GetVariable;

private:
  std::string_view                                          name;
  std::atomic_uint32_t                                      ref_count;
  std::list<size_t>                                         attach_ids;
  std::mutex                                                mtx;
  std::shared_ptr<shm_kernel::memory_manager::base_segment> segment;

public:
  variable_desc(const DTYPE    dtype,
                const size_t   size,
                const bool     is_bp,
                const uint32_t from_id,
                std::shared_ptr<shm_kernel::memory_manager::base_segment>);

  /**
   * @brief if a client id is not in attach_ids, it will append it, otherwise do
   * nothing.
   *
   * @param client_id
   */
  void add_attached_client(const uint32_t client_id) noexcept;

  libmem::segmentdesc get_segmentdesc() const noexcept;
};

class attached_variable : public base_variable
{

private:
  std::atomic_uint32_t                                     local_ref_count;
  std::shared_ptr<shm_kernel::memory_manager::segmentdesc> segment_descriptor;
  std::shared_ptr<shm_kernel::shared_memory::shm_handle>   shm_handle;
  void*                                                    variable_meta;
  void*                                                    variable_buffer;

public:
  attached_variable(std::shared_ptr<libmem::segmentdesc>,
                    std::shared_ptr<shm_kernel::shared_memory::shm_handle>,
                    const variable_desc* var,
                    const size_t         meta_pshift,
                    const size_t         buffer_pshift);

  attached_variable(const RESP_VariableShmInsert*);
};

class cached_variable : public base_variable
{
  void* buffer;

  cached_variable(const RESP_VariableCacheInsert*, const void* buff);
};

} // namespace shmpy
