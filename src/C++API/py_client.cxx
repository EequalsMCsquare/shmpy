#include "py_client.hpp"
#include "py_basepool.hpp"
#include "py_message.hpp"
#include <endpoint/base_client.hpp>
#include <endpoint/base_handler.hpp>
#include <memory>
#include <msg/base_msg.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

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
  this->_M_Id = ++(this->meta_ptr_->client_counter);
  spdlog::trace("客户端_{} Pool Meta初始化完毕!", this->id());
}

void
Py_Client::init_CALLBACKS()
{
  logger_->trace("正在初始化客户端Pool的Callbacks");
  msgclt_->set_callback(
    REQ_Detach::MSG_TYPE,
    [this](zmqmsg_iter __begin, zmqmsg_iter __end) -> callback_returns {
      logger_->trace("Callback <REQ_DETACH>");
      msg_head*   __recv_head = __begin->data<msg_head>();
      REQ_Detach* __recv_body = (__begin + 1)->data<REQ_Detach>();

      if (__recv_head->from != 0) {
        logger_->error(
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
      }

      logger_->trace("Callback <REQ_DETACH> Success!");
      this->pool_status_ = POOL_STATUS::TERMINATE;
      return callback_returns::success;
    });
  logger_->trace("客户端Pool的Callbacks初始化完毕!");
}

void
Py_Client::reply_fail(const uint32_t   to,
                      const int        req_type,
                      std::string_view why)
{
  msg_head     __send_head(this->id(), to, RESP_Failure::MSG_TYPE, 1);
  RESP_Failure __send_body(why);
  msgclt_->send(to,
                { zmq::buffer(&__send_head, sizeof(msg_head)),
                  zmq::buffer(&__send_body, sizeof(RESP_Failure)) });
}

Py_Client::Py_Client(std::string_view pool_name)
  : Py_BasePool(pool_name)
{
  spdlog::trace("正在初始化共享内存客户端...");
  this->init_META();
  this->logger_ = spdlog::stdout_color_mt(
    fmt::format("{}/client_{}", pool_name, this->_M_Id));
  this->msgclt_ = std::make_unique<libmsg::base_client>(
    this->_M_Id, meta_ptr_->zmq_send_port, meta_ptr_->zmq_recv_port, logger_);
  this->init_CALLBACKS();
  this->pool_status_ = POOL_STATUS::OK;
  logger_->trace("共享内存客户端初始化完毕!");
}

Py_Client::~Py_Client()
{
  logger_->trace("正在清理客户端Pool");
  logger_->trace("客户端Pool清理完毕!");
}

void
Py_Client::Py_IntInsert(std::string_view name, const py::int_& number)
{
  // TODO:
}

py::int_
Py_Client::Py_IntGet(std::string_view name)
{
  // TODO:
}

void
Py_Client::Py_IntSet(std::string_view name, const py::int_& number)
{
  // TODO:
}
uint32_t
Py_Client::id() const noexcept
{
  return this->_M_Id;
}

std::string_view
Py_Client::Py_Name() const noexcept
{
  return this->pool_name_;
}
uint32_t
Py_Client::Py_Id() const noexcept
{
  return msgclt_->id();
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