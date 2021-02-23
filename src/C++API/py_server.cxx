#include "py_server.hpp"
#include "py_common.hpp"
#include "py_config.hpp"
#include "py_except.hpp"
#include "py_message.hpp"

#include <memory>
#include <mutex>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef WIN32
#include <process.h>
#define getpid _getpid
#elif defined(UNIX)
#include <unistd.h>
#endif

namespace shmpy {

Py_Server::Py_Server(std::string_view name)
  : name_(name)
{
  this->logger_ = spdlog::stdout_color_mt(fmt::format("{}/server", name));

  logger_->trace("initializing Py_Server ...");
  this->mmgr_ = std::make_unique<mmgr_t>(
    this->name_, config::BatchBinSize, config::BatchBinCount, this->logger_);
  this->msgsvr_ = std::make_unique<msgsvr_t>(this->logger_);
  this->init_META();
  this->pool_status_ = POOL_STATUS::OK;
  logger_->trace("Py_Server initialize complete!");
}

Py_Server::~Py_Server()
{
  std::error_code ec;
  this->pool_status_ = POOL_STATUS::TERMINATE;
  this->meta_->ref_count--;

  std::lock_guard __lock(this->mtx_);
  if (msgsvr_->connected_client_ids().size() > 0) {
    logger_->trace("begin to close Py_Client ...");
    // 防止迭代器失效
    std::vector<uint32_t> __tmp_vec(msgsvr_->connected_client_ids());
    for (const auto& id : __tmp_vec) {
      this->Py_CloseClient(id, ec);
    }
    logger_->trace("Py_Client close complete!");
  }
  spdlog::drop(this->logger_->name());
}

void
Py_Server::Py_CloseClient(const uint32_t client_id, std::error_code& ec) noexcept
{
  ec.clear();
  logger_->trace("begin to close Py_Client_{} ...", client_id);
  this->msgsvr_->command(client_id, static_cast<uint32_t>(Py_Commands::detach_pool));
  msgsvr_->close_client(client_id, ec);
  if (ec) {
    logger_->error(
      "fail to close client socket! {}({}) {}", ec.category().name(), ec.value(), ec.message());
  }
}

void
Py_Server::init_CALLBACKS()
{}

void
Py_Server::init_META()
{
  logger_->trace("begin to intialize Py_Server's pool meta ...");
  std::error_code ec;
  this->meta_shmhdl_ =
    std::make_shared<shm_t>(fmt::format("shmpy#{}", this->name_), sizeof(POOL_META));

  // map pool meta
  this->meta_ = reinterpret_cast<POOL_META*>(this->meta_shmhdl_->map(ec));
  if (ec) {
    logger_->critical("unable to initialize Py_Server's pool meta! {}({}) {}",
                      ec.category().name(),
                      ec.value(),
                      ec.message());
    throw ShmpyExcept(ec);
  }

  // configure pool meta
  this->meta_->msgsvr_pub_port = this->msgsvr_->pub_port();
  this->meta_->msgsvr_rep_port = this->msgsvr_->rep_port();
  this->meta_->ref_count       = 1;
  this->meta_->owner_pid       = getpid();
  logger_->trace("Py_Server's pool meta initialize complete!");
}

uint32_t
Py_Server::id() const noexcept
{
  return this->msgsvr_->id();
}

uint32_t
Py_Server::Py_Id() const noexcept
{
  return this->msgsvr_->id();
}

std::string_view
Py_Server::Py_Name() const noexcept
{
  return this->name_;
}
POOL_STATUS
Py_Server::Py_Status() const noexcept
{
  return this->pool_status_;
}
uint32_t
Py_Server::Py_RefCount() const noexcept
{
  return this->meta_->ref_count;
}
}