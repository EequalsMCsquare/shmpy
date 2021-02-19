#include "py_basepool.hpp"
#include "py_client.hpp"
#include "py_config.hpp"
#include "py_server.hpp"
#include <catch2/catch.hpp>
#include <thread>
#include <unistd.h>

using namespace std::chrono_literals;

TEST_CASE("a server should be created without any exception", "[create]")
{
  std::string name = "PyServerName1";
  shmpy::Py_Server svr(name);
  REQUIRE(svr.Py_Name().compare(name) == 0);
  REQUIRE(svr.Py_Id() == 0);
  REQUIRE(svr.Py_Status() == shmpy::POOL_STATUS::OK);
  REQUIRE(svr.Py_RefCount() == 1);
}

TEST_CASE("create a server with an existing server should throw", "[create]")
{
  std::string name = "PyServerName2";
  shmpy::Py_Server svr1(name);

  REQUIRE_THROWS(shmpy::Py_Server(name));
  REQUIRE(svr1.Py_RefCount() == 1);
  REQUIRE(svr1.Py_Status() == shmpy::POOL_STATUS::OK);
}

SCENARIO("Py_Client is connecting to a Py_Server", "[connect]")
{
  GIVEN("a Py_Server")
  {
    std::string      __PyServerName = "PyServerName3";
    shmpy::Py_Server svr(__PyServerName);
    std::this_thread::sleep_for(200ms);

    WHEN("a Py_Client is connected")
    {
      {
        shmpy::Py_Client clt(__PyServerName);
        THEN("Py_Client id should be 1") { REQUIRE(clt.Py_Id() == 1); }
        THEN("Py_Client status should be OK")
        {
          REQUIRE(clt.Py_Status() == shmpy::POOL_STATUS::OK);
        }
        THEN("Both client and server's ref_count should increase and be equal")
        {
          REQUIRE(clt.Py_RefCount() == svr.Py_RefCount());
          REQUIRE(svr.Py_RefCount() == 2);
        }
        THEN("Py_Client name should match Py_Server's") { REQUIRE(clt.Py_Name() == svr.Py_Name()); }
        std::this_thread::sleep_for(200ms);
      }
      AND_WHEN("a Py_Client is out of scope(destroyed)")
      {
        THEN("Py_Server's ref_count decrease") { REQUIRE(svr.Py_RefCount() == 1); }
      }
      AND_WHEN("another Py_Client is connected")
      {
        shmpy::Py_Client clt(__PyServerName);

        THEN("ref count should be 2 for both Py_Client and Py_Server")
        {
          REQUIRE(clt.Py_RefCount() == 2);
          REQUIRE(svr.Py_RefCount() == 2);
        }
        THEN("new Py_Client's id should be 2") { REQUIRE(clt.Py_Id() == 2); }
        std::this_thread::sleep_for(200ms);
      }
      AND_WHEN("a Py_Client is out of scope(destroyed)")
      {
        THEN("Py_Server's ref_count decrease") { REQUIRE(svr.Py_RefCount() == 1); }
      }
      AND_WHEN("another Py_Client is connected")
      {
        shmpy::Py_Client clt(__PyServerName);

        THEN("ref count should be 2 for both Py_Client and Py_Server")
        {
          REQUIRE(clt.Py_RefCount() == 2);
          REQUIRE(svr.Py_RefCount() == 2);
        }
        THEN("new Py_Client's id should be 3") { REQUIRE(clt.Py_Id() == 3); }
        std::this_thread::sleep_for(200ms);
      }
      AND_WHEN("a Py_Client is out of scope(destroyed)")
      {
        THEN("Py_Server's ref_count decrease") { REQUIRE(svr.Py_RefCount() == 1); }
      }
    }

    WHEN("10 Py_Client is connecting to Py_Server")
    {
      std::vector<std::unique_ptr<shmpy::Py_Client>> clts;
      clts.reserve(10);
      size_t i;
      for (i = 0; i < 10; i++) {
        clts.emplace_back(std::make_unique<shmpy::Py_Client>(__PyServerName));
        THEN("Py_Server and Py_Client's ref_count should increase")
        {
          REQUIRE(svr.Py_RefCount() == i + 2);
          REQUIRE(clts[i]->Py_RefCount() == i + 2);
        }
        THEN("Py_Client Id should increase") { REQUIRE(clts[i]->Py_Id() == i + 1); }
      }
      THEN("now, the ref_count should be 11")
      {
        REQUIRE(svr.Py_RefCount() == 11);
        for (const auto& clt : clts) {
          REQUIRE(clt->Py_RefCount() == 11);
        }
      }
      AND_WHEN("the 10 Py_Client are destroyed")
      {
        std::this_thread::sleep_for(200ms);
        clts.clear();
        THEN("the vector size should be 0") { REQUIRE(clts.size() == 0); }
        THEN("Py_Server's ref_count should be 1") { REQUIRE(svr.Py_RefCount() == 1); }
      }
      AND_WHEN("a new Py_Client is connected")
      {
        shmpy::Py_Client clt(__PyServerName);
        THEN("ref_count should increase")
        {
          REQUIRE(svr.Py_RefCount() == 2);
          REQUIRE(clt.Py_RefCount() == 2);
        }
        std::this_thread::sleep_for(200ms);
        THEN("new Py_Client's Id should be 11") { REQUIRE(clt.Py_Id() == 11); }
      }
    }
  }
}

TEST_CASE("Py_Server is destroyed before Py_Client", "[destruct]")
{
  std::string __PyServerName = "PyServerName4";
  auto        svr            = new shmpy::Py_Server(__PyServerName);
  shmpy::Py_Client clt(__PyServerName);
  REQUIRE(clt.Py_Id() == 1);

  REQUIRE(clt.Py_RefCount() == 2);
  REQUIRE(svr->Py_RefCount() == 2);
  delete svr;
  REQUIRE(clt.Py_Status() == shmpy::POOL_STATUS::TERMINATE);
  REQUIRE_THROWS_AS(clt.Py_RefCount(), std::runtime_error);
  REQUIRE_THROWS_AS(clt.Py_OwnerPid(), std::runtime_error);
}

TEST_CASE("Py_Server is manually close Py_Client", "[Py_CloseClient]")
{
  std::string      __PyServerName = "PyServerName5";
  shmpy::Py_Server svr(__PyServerName);
  shmpy::Py_Client clt(__PyServerName);

  svr.Py_CloseClient(clt.Py_Id());

  REQUIRE(svr.Py_RefCount() == 1);
  REQUIRE(clt.Py_Status() == shmpy::POOL_STATUS::TERMINATE);
  REQUIRE_THROWS_AS(clt.Py_RefCount(), std::runtime_error);
}
