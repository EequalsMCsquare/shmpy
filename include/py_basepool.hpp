#pragma once

#include <atomic>
#include <cstddef>
#include <ctime>
#include <endpoint/base_handler.hpp>
#include <map>
#include <memory>
#include <regex>

#ifdef WIN32
#include <process.h>
#define getpid _getpid
#elif defined(UNIX)
#include <unistd.h>
#endif

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
namespace py     = pybind11;

enum class POOL_STATUS
{
  OK,       // Pool is OK
  DETACH,   // Pool is detached
  TERMINATE // Pool is terminated.
};

struct META
{
  pid_t                owner_pid;
  uint32_t             zmq_send_port;
  uint32_t             zmq_recv_port;
  std::atomic_uint32_t client_counter;
  std::atomic_uint32_t ref_count;
};

class Py_BasePool
{

protected:
  using callback_returns = shm_kernel::message_handler::callback_returns;
  using msg_head         = shm_kernel::message_handler::msg_head;
  using zmqmsg_iter      = shm_kernel::message_handler::zmqmsg_iter;

  inline static auto TRACE_RX =
    std::regex{ "trace", std::regex_constants::icase };
  inline static auto DEBUG_RX =
    std::regex{ "debug", std::regex_constants::icase };
  inline static auto INFO_RX =
    std::regex{ "info", std::regex_constants::icase };
  inline static auto WARN_RX =
    std::regex{ "warn", std::regex_constants::icase };
  inline static auto ERROR_RX =
    std::regex{ "error", std::regex_constants::icase };
  inline static auto CRITICAL_RX =
    std::regex{ "critical", std::regex_constants::icase };

  uint32_t                                              _M_Id;
  std::string                                           pool_name_;
  POOL_STATUS                                           pool_status_;
  std::shared_ptr<libshm::shm_handle>                   meta_handle_;
  std::mutex                                            mtx_;
  META*                                                 meta_ptr_;
  std::map<std::string, attached_variable, std::less<>> attched_variables_;
  std::shared_ptr<spdlog::logger>                       logger_;

  virtual void reply_fail(const uint32_t   to,
                          const int        req_type,
                          std::string_view why) = 0;

  virtual void     init_CALLBACKS()    = 0;
  virtual void     init_META()         = 0;
  virtual uint32_t id() const noexcept = 0;

  virtual void Py_IntInsert(std::string_view name, const py::int_& number) = 0;
  // virtual void PyFloat_INSERT(std::string_view  name,
  //                             const py::float_& number)                    =
  //                             0;
  // virtual void PuBuff_INSERT(std::string_view name, const py::buffer &buff) =
  // 0; virtual void PyList_INSERT(std::string_view name, const py::list &list)
  // = 0; virtual void PyPickle_INSERT(std::string_view name,
  //                              const py::object &obj) = 0;

  virtual void Py_IntSet(std::string_view name, const py::int_& number) = 0;
  // virtual void PyFloat_SET(std::string_view name, const py::float_ &number) =
  // 0; virtual void PyBuff_SET(std::string_view name, const py::buffer &buffer)
  // = 0; virtual void PyList_SET(std::string_view name, const py::list &list) =
  // 0; virtual void PyPickle_SET(std::string_view name, const py::object &obj)
  // = 0;

  virtual py::int_ Py_IntGet(std::string_view name) = 0;
  // virtual py::float_ PyFloat_GET(std::string_view name) = 0;
  // virtual py::buffer PyBuff_GET(std::string_view name) = 0;
  // virtual py::list PyList_GET(std::string_view name) = 0;
  // virtual py::object PyPickle_GET(std::string_view name) = 0;

  // virtual void PyVariable_DEL(std::string_view name) = 0;

  static std::string MAKE_META_HANDLE_NAME(std::string_view name);

public:
  explicit Py_BasePool(std::string_view pool_name);
  // virtual ~Py_BasePool();
  void set_log_level(const std::string& level);

  // virtual void insert(const std::string_view name, const py::object &var);
  // virtual void set(const std::string_view name, const py::object &var);
  // virtual py::object get(const std::string_view name);
  // virtual void del(std::string_view name);

  // virtual const auto &Py_AttachedVariables() const noexcept = 0;

  virtual std::string_view Py_Name() const noexcept     = 0;
  virtual uint32_t         Py_Id() const noexcept       = 0;
  virtual uint32_t         Py_RefCount() const noexcept = 0;
  virtual POOL_STATUS      Py_Status() const noexcept   = 0;
  virtual pid_t            Py_OwnerPid() const noexcept = 0;
};
} // namespace shmpy
