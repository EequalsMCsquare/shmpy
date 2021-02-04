#pragma once

#include <bits/types/FILE.h>
#include <endpoint/base_client.hpp>
#include <segment.hpp>

#include <memory>
#include <memory_resource>
#include <spdlog/spdlog.h>

#include "py_basepool.hpp"

namespace shmpy {

struct Py_Client : public Py_BasePool
{

private:
  std::unique_ptr<libmsg::base_client>                                 msgclt_;
  std::map<std::string, std::shared_ptr<cached_variable>, std::less<>> cached_variables_;
  std::pmr::unsynchronized_pool_resource                               pmr_pool_;

  void     init_META() override;
  void     init_CALLBACKS() override;
  void     reply_fail(const uint32_t to, const int req_type, std::string_view why) override;
  uint32_t id() const noexcept final override;

public:
  explicit Py_Client(std::string_view pool_name);
  ~Py_Client();

  void     Py_IntInsert(std::string_view  name,
                        const py::int_&   number,
                        const ACCESS_TYPE access_type) override final;
  py::int_ Py_IntGet(std::string_view name) override final;
  void     Py_IntSet(std::string_view name, const py::int_& number) override final;

  void       Py_FloatInsert(std::string_view  name,
                            const py::float_& number,
                            const ACCESS_TYPE access_type) override final;
  py::float_ Py_FloatGet(std::string_view name) override final;
  void       Py_FloatSet(std::string_view name, const py::float_& number) override final;

  void      Py_BoolInsert(std::string_view  name,
                          const py::bool_&  boolean,
                          const ACCESS_TYPE access_type) override final;
  py::bool_ Py_BoolGet(std::string_view name) override final;
  void      Py_BoolSet(std::string_view name, const py::bool_& boolean) override final;

  void    Py_StrInsert(std::string_view  name,
                       std::string_view  str,
                       const ACCESS_TYPE access_type) override final;
  void    Py_StrSet(std::string_view name, std::string_view str) override final;
  py::str Py_StrGet(std::string_view name) override final;

  void       Py_GenericInsert(std::string_view name, const py::object& obj) override final;
  void       Py_GenericDelete(std::string_view name) override final;
  void       Py_GenericSet(std::string_view name, const py::object& obj) override final;
  py::object Py_GenericGet(std::string_view name) override final;

  std::string_view Py_Name() const noexcept override;
  uint32_t         Py_Id() const noexcept override;
  uint32_t         Py_RefCount() const noexcept final override;
  POOL_STATUS      Py_Status() const noexcept final override;
  pid_t            Py_OwnerPid() const noexcept final override;
};
} // namespace shmpy
