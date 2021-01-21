#pragma once

#include <atomic>
#include <cstddef>
#include <ctime>
#include <map>
#include <memory>

#ifdef WIN32
#include <process.h>
#define getpid _getpid
#elif defined(UNIX)
#include <unistd.h>
#endif

#include <endpoint/base_handler.hpp>
#include <mem_literals.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <segment.hpp>
#include <shared_memory.hpp>
#include <spdlog/logger.h>
#include <string>
#include <string_view>

#include "py_variable.hpp"

namespace shmpy {

namespace libshm = shm_kernel::shared_memory;
namespace libmem = shm_kernel::memory_manager;
namespace libmsg = shm_kernel::message_handler;

enum class POOL_STATUS
{
  OK,       // Pool is OK
  DETACH,   // Pool is detached
  TERMINATE // Pool is terminated.
};

struct META
{
  pid_t owner_pid;
  uint32_t zmq_send_port;
  uint32_t zmq_recv_port;
  std::atomic_uint32_t ref_count;
};

class Py_BasePool
{

private:
  std::shared_ptr<spdlog::logger> logger_;
  virtual void init_CALLBACKS() = 0;

protected:
  std::string pool_name_;
  POOL_STATUS pool_status_;
  std::shared_ptr<libshm::shm_handle> meta_handle_;
  std::mutex mtx_;
  META* meta_ptr_;
  std::map<std::string, attached_variable, std::less<>> attched_variables_;

  virtual void PyInt_SET(std::string_view name, const py::int_& number) = 0;
  virtual void PyFloat_SET(std::string_view name, const py::float_& number) = 0;

  //  virtual void PyBuff_SET(std::string_view name, const py::buffer& buffer) =
  //  0; virtual void PyList_SET(std::string_view name, const py::list& list) =
  //  0; virtual void Pystring_view_SET(std::string_view name, const py::str&
  //  str) = 0; virtual void PyPickle_SET(std::string_view name, const
  //  py::object& obj) = 0;

public:
  explicit Py_BasePool(const std::string& pool_name,
                       std::shared_ptr<spdlog::logger> __logger)
    : pool_name_(pool_name)
    , logger_(__logger)
  {}

  virtual void insert(const std::string_view name, const py::object& var)
  {
    // Try to find the name in attached_variables_
    auto __iter = this->attched_variables_.find(name);
    if (__iter != this->attched_variables_.end()) {
      this->logger_->error("变量的名称必须是唯一的.");
      throw std::runtime_error(
        "variable name already exist in local's attached_variables. If you "
        "want to modify the variable, please use set(...) instead.");
    }
    // Type checking
    // if variable is np.ndarray
    if (py::isinstance(var, py::module_::import("numpy").attr("ndarray"))) {
      // TODO:
    } else if (py::isinstance(var, py::int_())) {
      this->PyInt_SET(name, var);
    } else if (py::isinstance(var, py::float_())) {
      this->PyFloat_SET(name, var);
    }
  }

  virtual void set(const std::string_view name, const py::object& var)
  {
    // TODO:
  }

  virtual py::object get(const std::string_view name)
  {
    // TODO:
  }

  virtual std::string_view name() noexcept = 0;
};
} // namespace shmpy
