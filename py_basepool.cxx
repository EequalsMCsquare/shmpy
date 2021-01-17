#pragma once

#include <atomic>
#include <cstddef>
#include <ctime>
#include <map>
#include <memory>

#include <mem_literals.hpp>
#include <segment.hpp>
#include <shared_memory.hpp>
#include <string>

namespace shmpy {

namespace libshm = shm_kernel::shared_memory;
namespace libmem = shm_kernel::memory_manager;

enum class POOL_STATUS {
  OK,       // Pool is OK
  DETACH,   // Pool is detached
  TERMINATE // Pool is terminated.
};

struct META {
  pid_t owner_pid;
  uint32_t zmq_send_port;
  uint32_t zmq_recv_port;
  std::atomic_uint32_t ref_count;
};

class Py_BasePool {

protected:
  size_t id_;
  POOL_STATUS pool_status_;
  std::shared_ptr<libshm::shm_handle> meta_handle_;
  META *meta_ptr_;
};
} // namespace shmpy