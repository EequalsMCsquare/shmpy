#pragma once

#include <bits/types/FILE.h>
#include <endpoint/base_server.hpp>
#include <mmgr.hpp>

#include "py_basepool.hpp"
#include "py_config.hpp"
#include "py_message.hpp"

namespace shmpy {
using namespace std::chrono_literals;
class Py_Server
  : public libmsg::base_server
  , public Py_BasePool
  , public libmem::mmgr
{

private:
  std::atomic_size_t segment_counter_{ 0 };
  std::map<std::string, std::shared_ptr<variable_desc>, std::less<>>
    variable_table_;

  void init_CALLBACKS() override;
  void init_META() override;
  void reply_fail(const uint32_t   to,
                  const int        req_type,
                  std::string_view why) override;

  // void PyVariable_DEL(std::string_view name) final;

public:
  explicit Py_Server(std::string_view name);
  ~Py_Server();
  void Py_CloseClient(const uint32_t client_id,
                      const bool     terminate_sock,
                      const bool     detach_batch   = true,
                      const bool     detach_instant = true,
                      const bool     detach_meta    = true);

  size_t Py_CacheBinEps() const noexcept;
  size_t Py_InstantBinEps() const noexcept;

  std::string_view           Py_Name() const noexcept final override;
  uint32_t                   Py_Id() const noexcept final override;
  uint32_t                   Py_RefCount() const noexcept final override;
  POOL_STATUS                Py_Status() const noexcept final override;
  pid_t                      Py_OwnerPid() const noexcept final override;
  const std::list<uint32_t>& Py_ClientIds() const noexcept;
};
} // namespace shmpy
