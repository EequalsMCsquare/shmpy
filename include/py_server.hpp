#pragma once

#include <bits/types/FILE.h>
#include <endpoint/base_server.hpp>
#include <memory>
#include <mmgr.hpp>
#include <pybind11/pytypes.h>

#include "py_basepool.hpp"
#include "py_config.hpp"
#include "py_message.hpp"
#include "py_variable.hpp"

namespace shmpy {
using namespace std::chrono_literals;
class Py_Server : public Py_BasePool
{

private:
  std::atomic_size_t                   segment_counter_{ 0 };
  std::unique_ptr<libmsg::base_server> msgsvr_;
  std::unique_ptr<libmem::mmgr>        mmgr_;
  std::map<std::string, std::shared_ptr<variable_desc>, std::less<>>
    variable_table_;

  uint32_t id() const noexcept final override;
  void     init_CALLBACKS() override;
  void     init_META() override;
  void     reply_fail(const uint32_t   to,
                      const int        req_type,
                      std::string_view why) override;

  void HANDLE_IntInsert(std::string_view name,
                        const long       number,
                        const uint32_t   inserter_id);

  void HANDLE_IntSet(std::string_view name,
                     const long       number,
                     const uint32_t   setter_id);

  std::optional<long> HANDLE_IntGet(std::string_view name,
                                    const uint32_t   requester_id);

public:
  explicit Py_Server(std::string_view name);
  ~Py_Server();
  void Py_CloseClient(const uint32_t client_id,
                      const bool     terminate_sock,
                      const bool     detach_batch   = true,
                      const bool     detach_instant = true,
                      const bool     detach_meta    = true);

  void Py_IntInsert(std::string_view name,
                    const py::int_&  number) override final;

  py::int_ Py_IntGet(std::string_view name) override final;

  void Py_IntSet(std::string_view name, const py::int_& number) override final;

  size_t                     Py_CacheBinEps() const noexcept;
  size_t                     Py_InstantBinEps() const noexcept;
  std::string_view           Py_Name() const noexcept final override;
  uint32_t                   Py_Id() const noexcept final override;
  uint32_t                   Py_RefCount() const noexcept final override;
  POOL_STATUS                Py_Status() const noexcept final override;
  pid_t                      Py_OwnerPid() const noexcept final override;
  const std::list<uint32_t>& Py_ClientIds() const noexcept;
};

} // namespace shmpy
