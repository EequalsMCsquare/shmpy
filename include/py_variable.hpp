#pragma once

#include "py_dtype.hpp"
#include "shared_memory.hpp"

#include <segment.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>

namespace shmpy {

enum class VAR_STATUS
{
  OK,     //
  DETACH, //
  AB      // 这是啥玩意啊....之前忘记写注释了。操
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
  DTYPE dtype;
  ACCESS_TYPE access_type;
  bool is_py_buff_protocol;
  size_t size;
};

/**
 * @brief 只保存在服务器端
 *
 */
class variable_desc : public base_variable
{
private:
  std::string_view name;
  std::atomic_uint32_t ref_count;
  std::list<size_t> attach_ids;
  std::mutex mtx;
  std::shared_ptr<shm_kernel::memory_manager::base_segment> segment;
};

class attached_variable : public base_variable
{
private:
  std::atomic_uint32_t local_ref_count;
  std::shared_ptr<shm_kernel::memory_manager::segmentdesc> segment_descriptor;
  std::shared_ptr<shm_kernel::shared_memory::shm_handle> shm_handle;
  void* variable_meta;
  void* variable_buffer;
};
} // namespace shmpy
