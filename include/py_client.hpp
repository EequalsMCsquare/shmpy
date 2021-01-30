#pragma once

#include <bits/types/FILE.h>
#include <endpoint/base_client.hpp>
#include <segment.hpp>

#include <memory>
#include <spdlog/spdlog.h>

#include "py_basepool.hpp"

namespace shmpy {

struct Py_Client : public Py_BasePool
{

private:
  std::unique_ptr<libmsg::base_client> msgclt_;

  void     init_META() override;
  void     init_CALLBACKS() override;
  void     reply_fail(const uint32_t   to,
                      const int        req_type,
                      std::string_view why) override;
  uint32_t id() const noexcept final override;

public:
  explicit Py_Client(std::string_view pool_name);
  ~Py_Client();

  void Py_IntInsert(std::string_view name,
                    const py::int_&  number) override final;

  py::int_ Py_IntGet(std::string_view name) override final;

  void Py_IntSet(std::string_view name, const py::int_& number) override final;

  std::string_view Py_Name() const noexcept override;
  uint32_t         Py_Id() const noexcept override;
  uint32_t         Py_RefCount() const noexcept final override;
  POOL_STATUS      Py_Status() const noexcept final override;
  pid_t            Py_OwnerPid() const noexcept final override;
};
} // namespace shmpy
