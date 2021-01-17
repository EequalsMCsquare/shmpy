#pragma once

#include "py_dtype.cxx"
#include "shared_memory.hpp"

#include <segment.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>

namespace shmpy {

enum class VAR_STATUS {
  OK,     //
  DETACH, //
  AB      // 这是啥玩意啊。。。
};

class base_variable {
protected:
  DTYPE dtype;
  bool is_py_buff_protocol;
  size_t size;
};

class variable_desc : public base_variable {
private:
  std::atomic_uint32_t ref_count;
  std::list<size_t> attach_ids;
  std::mutex mtx;
  std::shared_ptr<shm_kernel::memory_manager::base_segment> segment;
};

class attached_variable : public base_variable {
private:
  std::atomic_uint32_t local_ref_count;
  std::shared_ptr<shm_kernel::memory_manager::segmentdesc> segment_descriptor;
  std::shared_ptr<shm_kernel::shared_memory::shm_handle> shm_handle;
};
} // namespace shmpy