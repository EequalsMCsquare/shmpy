#include "py_dtype.hpp"
#include "py_server.hpp"
#include <catch2/catch.hpp>
#include <thread>

using namespace std::chrono_literals;
using DTYPE = shmpy::DTYPE;

TEST_CASE("a server should be created without any exception", "[create]")
{
  std::string      name = "PyServerName1";
  shmpy::Py_Server svr(name);
  REQUIRE(svr.Py_Name().compare(name) == 0);
  REQUIRE(svr.Py_Id() == 0);
  REQUIRE(svr.Py_Status() == shmpy::POOL_STATUS::OK);
  REQUIRE(svr.Py_RefCount() == 1);
  REQUIRE(svr.variable_table_.size() == 0);
}

TEST_CASE("server HANDLE_CacheInsert", "[cache]")
{
  std::error_code  ec;
  std::string      name = "PyServerName2";
  shmpy::Py_Server svr(name);
  int32_t          __var1 = 100;
  svr.HANDLE_CacheInsert<DTYPE::int32>(
    "var1",
    sizeof(int32_t),
    svr.Py_Id(),
    [&__var1](void* buffer) { *static_cast<int32_t*>(buffer) = __var1; },
    ec);
  REQUIRE(svr.variable_table_.size() == 1);
  auto __var1_iter = svr.variable_table_.find("var1");
  REQUIRE(__var1_iter != svr.variable_table_.end());
  REQUIRE(__var1_iter->first.compare("var1") == 0);
  auto __var1_desc = __var1_iter->second;
  REQUIRE(__var1_desc->attach_ids_.size() == 0);
  REQUIRE(__var1_desc->size_ == sizeof(int32_t));
  REQUIRE(__var1_desc->ref_count_ == 1);
  REQUIRE(__var1_desc->name_.compare(__var1_iter->first) == 0);
  REQUIRE(__var1_desc->dtype_ == DTYPE::int32);
  REQUIRE(__var1_desc->attach_ids_.front() == svr.Py_Id());
}