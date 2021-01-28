#pragma once

#include <endpoint/base_client.hpp>
#include <segment.hpp>

#include <memory>
#include <spdlog/spdlog.h>

#include "py_basepool.hpp"

namespace shmpy {

struct Py_Client
  : public libmsg::base_client
  , public Py_BasePool
{

private:
  void init_META() override;
  void init_CALLBACKS() override;
  void reply_fail(const uint32_t   to,
                  const int        req_type,
                  std::string_view why) override;

public:
  explicit Py_Client(std::string_view pool_name);
  ~Py_Client();

  std::string_view Py_Name() const noexcept override;
  uint32_t         Py_Id() const noexcept override;

  uint32_t    Py_RefCount() const noexcept final override;
  POOL_STATUS Py_Status() const noexcept final override;
  pid_t       Py_OwnerPid() const noexcept final override;
};
} // namespace shmpy
