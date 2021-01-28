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

class base_variable
{
protected:
  DTYPE       dtype;
  ACCESS_TYPE access_type;
  bool        is_pybuff_protocol;
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
  variable_desc(const shm_kernel::message_handler::msg_head* msg_head,
                const REQ_InsertVariable*                    req,
                std::shared_ptr<shm_kernel::memory_manager::base_segment>);
};

class attached_variable : public base_variable
{
private:
  std::atomic_uint32_t                                     local_ref_count;
  std::shared_ptr<shm_kernel::memory_manager::segmentdesc> segment_descriptor;
  std::shared_ptr<shm_kernel::shared_memory::shm_handle>   shm_handle;
  void*                                                    variable_meta;
  void*                                                    variable_buffer;
};

} // namespace shmpy
