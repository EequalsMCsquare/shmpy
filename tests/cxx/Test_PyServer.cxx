#include "py_basepool.hpp"
#include "py_client.hpp"
#include "py_config.hpp"
#include <unistd.h>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <py_server.hpp>

TEST_CASE("创建共享内存服务器并检测成员是否如预期初始化", "[PyServer]")
{
  shmpy::Py_Server svr("test");
  REQUIRE(svr.Py_Name() == "shmpy#test");
  REQUIRE(svr.Py_CacheBinEps() == shmpy::config::CacheBinEps);
  REQUIRE(svr.Py_InstantBinEps() == shmpy::config::InstantBinEps);
  REQUIRE(svr.Py_Id() == 0);
  REQUIRE(svr.Py_Status() == shmpy::POOL_STATUS::OK);
  REQUIRE(svr.Py_RefCount() == 1);
  REQUIRE(svr.Py_OwnerPid() == getpid());
}

TEST_CASE("客户端连接到一个服务端", "[PyServer]")
{
  shmpy::Py_Server svr("test2");
  shmpy::Py_Client clt("test2");
  REQUIRE(svr.Py_Id() == 0);
  REQUIRE(clt.Py_Id() == 1);
  REQUIRE(svr.Py_RefCount() == 2);
  REQUIRE(clt.Py_RefCount() == 2);
}