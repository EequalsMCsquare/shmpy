#include "py_basepool.hpp"
#include "py_client.hpp"
#include "py_config.hpp"
#include <spdlog/spdlog.h>
#include <thread>
#include <unistd.h>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <py_server.hpp>

using namespace std::chrono_literals;

TEST_CASE("创建共享内存服务器并检测成员是否如预期初始化", "[PyServer]")
{
  shmpy::Py_Server svr("test");
  REQUIRE(svr.Py_Name() == "test");
  REQUIRE(svr.Py_CacheBinEps() == shmpy::config::CacheBinEps);
  REQUIRE(svr.Py_InstantBinEps() == shmpy::config::InstantBinEps);
  REQUIRE(svr.Py_Id() == 0);
  REQUIRE(svr.Py_Status() == shmpy::POOL_STATUS::OK);
  REQUIRE(svr.Py_RefCount() == 1);
  REQUIRE(svr.Py_OwnerPid() == getpid());
  std::this_thread::sleep_for(200ms);
}

TEST_CASE("客户端连接到一个服务端", "[PyServer]")
{
  shmpy::Py_Server svr("test2");
  shmpy::Py_Client clt("test2");
  REQUIRE(svr.Py_Id() == 0);
  REQUIRE(clt.Py_Id() == 1);
  REQUIRE(svr.Py_RefCount() == 2);
  REQUIRE(clt.Py_RefCount() == 2);
  std::this_thread::sleep_for(200ms);
}

TEST_CASE("服务端手动关闭客户端", "[PyServer]")
{
  shmpy::Py_Server svr("test3");
  shmpy::Py_Client clt1("test3");
  REQUIRE(svr.Py_RefCount() == 2);
  svr.Py_CloseClient(1, true);
  // 延迟,解决方法 Close里阻塞等待回复
  CHECK(svr.Py_RefCount() == 1);
  REQUIRE(clt1.Py_Status() == shmpy::POOL_STATUS::TERMINATE);
  std::this_thread::sleep_for(200ms);
}

TEST_CASE("服务器先被Delete", "[PyServer]")
{
  auto svr = new shmpy::Py_Server("test4");
  auto clt = new shmpy::Py_Client("test4");
  delete svr;
  delete clt;
}