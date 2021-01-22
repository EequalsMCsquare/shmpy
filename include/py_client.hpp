#pragma once

#include <endpoint/base_client.hpp>
#include <segment.hpp>

#include <memory>
#include <spdlog/spdlog.h>

#include "py_basepool.hpp"

namespace shmpy {

struct Py_Client
  : Py_BasePool
  , libmsg::base_client
{

public:
  explicit Py_Client(const std::string& pool_name)
    : Py_BasePool(pool_name)
    , libmsg::base_client()
  {
    spdlog::trace("正在初始化共享内存客户端...");
    this->init_META();
    this->set_recv_ep(this->meta_ptr_->zmq_recv_port);
    this->set_send_ep(this->meta_ptr_->zmq_send_port);
    this->run();

    this->pool_status_ = POOL_STATUS::OK;
    logger->trace("共享内存客户端初始化完毕!");
  }

private:
  void init_META() override
  {
    spdlog::trace("正在初始化客户端Pool Meta");
    std::error_code ec;
    this->meta_handle_ = std::make_shared<libshm::shm_handle>(
      MAKE_META_HANDLE_NAME(this->name()), sizeof(META));
    this->meta_ptr_ = reinterpret_cast<META*>(this->meta_handle_->map(ec));
    if (ec) {
      spdlog::critical(
        "无法初始化Py_Client的内存池Meta! ({}) {}", ec.value(), ec.message());
      throw std::runtime_error("fail to initialize pool meta!");
    }
    // increase ref_count;
    this->meta_ptr_->ref_count += 1;
    spdlog::trace("客户端Pool Meta初始化完毕!");
  }

  void init_CALLBACKS() override {}
};
} // namespace shmpy
