#include "py_client.hpp"
#include "py_basepool.hpp"
#include "py_message.hpp"
#include <endpoint/base_handler.hpp>
#include <msg/base_msg.hpp>

namespace shmpy {

using msg_head = libmsg::msg_head;

void
Py_Client::init_META()
{
  spdlog::trace("正在初始化客户端Pool Meta");
  std::error_code ec;
  this->meta_handle_ = std::make_shared<libshm::shm_handle>(
    MAKE_META_HANDLE_NAME(this->Py_Name()));
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

void
Py_Client::init_CALLBACKS()
{
  logger->trace("正在初始化客户端Pool的Callbacks");
  this->set_callback(
    REQ_Detach::MSG_TYPE,
    [this](zmqmsg_iter __begin, zmqmsg_iter __end) -> callback_returns {
      logger->trace("Callback <REQ_DETACH>");
      msg_head*   __recv_head = __begin->data<msg_head>();
      REQ_Detach* __recv_body = (__begin + 1)->data<REQ_Detach>();

      if (__recv_head->from != 0) {
        logger->error(
          "接收到了ServerClosing的信息，但是发送者并不是Server. 无视信息!");
        return callback_returns::do_nothing;
      }
      if (__recv_body->batch) {
        // TODO:
      }
      if (__recv_body->instant) {
        // TODO:
      }
      if (__recv_body->meta) {
        this->meta_ptr_->ref_count--;
        this->meta_handle_->unlink();
      }

      logger->trace("Callback <REQ_DETACH> Success!");
      this->pool_status_ = POOL_STATUS::TERMINATE;
      return callback_returns::success;
    });
  logger->trace("客户端Pool的Callbacks初始化完毕!");
}

void
Py_Client::reply_fail(const uint32_t   to,
                      const int        req_type,
                      std::string_view why)
{
  msg_head     __send_head(this->id_, to, RESP_Failure::MSG_TYPE, 1);
  RESP_Failure __send_body(why);
  this->send(to,
             { zmq::buffer(&__send_head, sizeof(msg_head)),
               zmq::buffer(&__send_body, sizeof(RESP_Failure)) });
}

Py_Client::Py_Client(std::string_view pool_name)
  : Py_BasePool(pool_name)
  , libmsg::base_client()
{
  spdlog::trace("正在初始化共享内存客户端...");
  this->init_META();
  this->set_recv_ep(this->meta_ptr_->zmq_recv_port);
  this->set_send_ep(this->meta_ptr_->zmq_send_port);
  this->run();
  this->init_CALLBACKS();
  this->pool_status_ = POOL_STATUS::OK;
  logger->trace("共享内存客户端初始化完毕!");
}

Py_Client::~Py_Client() {}

std::string_view
Py_Client::Py_Name() const noexcept
{
  return this->pool_name_;
}
uint32_t
Py_Client::Py_Id() const noexcept
{
  return this->id_;
}
uint32_t
Py_Client::Py_RefCount() const noexcept
{
  return this->meta_ptr_->ref_count;
}
POOL_STATUS
Py_Client::Py_Status() const noexcept
{
  return this->pool_status_;
}
pid_t
Py_Client::Py_OwnerPid() const noexcept
{
  return this->meta_ptr_->owner_pid;
}
} // namespace shmpy