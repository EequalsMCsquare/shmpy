#pragma once

#include <bits/types/FILE.h>
#include <endpoint/base_server.hpp>
#include <memory>
#include <mmgr.hpp>
#include <optional>
#include <pybind11/pytypes.h>
#include <segment.hpp>
#include <type_traits>

#include "py_basepool.hpp"
#include "py_config.hpp"
#include "py_dtype.hpp"
#include "py_message.hpp"
#include "py_variable.hpp"

namespace shmpy {
using namespace std::chrono_literals;
class Py_Server : public Py_BasePool
{

private:
  std::atomic_size_t                                                 segment_counter_{ 0 };
  std::unique_ptr<libmsg::base_server>                               msgsvr_;
  std::unique_ptr<libmem::mmgr>                                      mmgr_;
  std::map<std::string, std::shared_ptr<variable_desc>, std::less<>> variable_table_;
  size_t instant_bin_eps_{ config::InstantBinEps };

  uint32_t id() const noexcept final override;
  void     init_CALLBACKS() override;
  void     init_META() override;
  void     reply_fail(const uint32_t to, const int req_type, std::string_view why) override;

  void HANDLE_GenericCacheInsert(std::string_view name,
                                 const void*      buffer,
                                 const size_t     size,
                                 const DTYPE      dtype,
                                 const uint32_t   inserter_id);

  void HANDLE_GenericCacheSet(std::string_view name,
                              const void*      buffer,
                              const size_t     size,
                              const DTYPE      dtype,
                              const uint32_t   setter_id);

  void HANDLE_GenericCacheDel(std::string_view name, const bool force, const bool notify_attachers);

  template<typename T>
  std::optional<T> HANDLE_GenericCacheGet(std::string_view name, const uint32_t requester_id);

  // template<typename T>
  // std::enable_if_t<std::is_fundamental<T>::value, std::optional<T>> HANDLE_GenericCacheGet(
  //   std::string_view name,
  //   const uint32_t   requester_id);

  // void HANDLE_IntInsert(std::string_view name, const long number, const uint32_t inserter_id);
  // void HANDLE_IntSet(std::string_view name, const long number, const uint32_t setter_id);
  // std::optional<long> HANDLE_IntGet(std::string_view name, const uint32_t requester_id);

public:
  explicit Py_Server(std::string_view name);
  ~Py_Server();
  void Py_CloseClient(const uint32_t client_id,
                      const bool     terminate_sock,
                      const bool     detach_batch   = true,
                      const bool     detach_instant = true,
                      const bool     detach_meta    = true);

  void     Py_IntInsert(std::string_view name, const py::int_& number) override final;
  void     Py_IntSet(std::string_view name, const py::int_& number) override final;
  py::int_ Py_IntGet(std::string_view name) override final;

  void       Py_FloatInsert(std::string_view name, const py::float_& number) override final;
  void       Py_FloatSet(std::string_view name, const py::float_& number) override final;
  py::float_ Py_FloatGet(std::string_view name) override final;

  void      Py_BoolInsert(std::string_view name, const py::bool_& boolean) override final;
  void      Py_BoolSet(std::string_view name, const py::bool_& boolean) override final;
  py::bool_ Py_BoolGet(std::string_view name) override final;

  void             Py_StrInsert(std::string_view name, std::string_view str) override final;
  void             Py_StrSet(std::string_view name, std::string_view str) override final;
  std::string_view Py_StrGet(std::string_view name) override final;

  void Py_GenericCacheVarDel(std::string_view name,
                             const bool       force            = false,
                             const bool       notify_attachers = false);

  size_t                     Py_InstantBinEps() const noexcept;
  std::string_view           Py_Name() const noexcept final override;
  uint32_t                   Py_Id() const noexcept final override;
  uint32_t                   Py_RefCount() const noexcept final override;
  POOL_STATUS                Py_Status() const noexcept final override;
  pid_t                      Py_OwnerPid() const noexcept final override;
  const std::list<uint32_t>& Py_ClientIds() const noexcept;
};

} // namespace shmpy
