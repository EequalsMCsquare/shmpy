#include "py_server.hpp"
#include "py_basepool.hpp"
#include "py_config.hpp"
#include "py_message.hpp"
#include <endpoint/base_handler.hpp>
#include <memory>
#include <mmgr.hpp>
#include <mutex>
#include <segment.hpp>
#include <thread>
#include <utility>
#include <zmq.hpp>

namespace shmpy {

void
Py_Server::init_META()
{
  this->logger->trace("正在初始化服务端Pool Meta...");
  std::error_code ec;
  this->meta_handle_ = std::make_shared<libshm::shm_handle>(
    MAKE_META_HANDLE_NAME(this->Py_Name()), sizeof(META));
  // map pool meta
  this->meta_ptr_ = reinterpret_cast<META*>(this->meta_handle_->map(ec));
  // check if success
  if (ec) {
    this->logger->critical(
      "无法初始化Py_Server的内存池Meta! ({}) {}", ec.value(), ec.message());
    throw std::runtime_error("fail to initialize pool meta!");
  }
  // configure pool meta
  this->meta_ptr_->zmq_recv_port = this->recv_port();
  this->meta_ptr_->zmq_send_port = this->send_port();
  this->meta_ptr_->ref_count     = 1;
  this->meta_ptr_->owner_pid     = getpid();
  this->logger->trace("服务端Pool Meta初始化完毕!");
}

void
Py_Server::reply_fail(const uint32_t   to,
                      const int        req_type,
                      std::string_view why)
{
  msg_head     __send_head(this->id_, to, RESP_Failure::MSG_TYPE, 1);
  RESP_Failure __send_body(why);
  this->send(to,
             { zmq::buffer(&__send_head, sizeof(msg_head)),
               zmq::buffer(&__send_body, sizeof(RESP_Failure)) });
}

void
Py_Server::init_CALLBACKS()
{
  logger->trace("正在初始化服务端Pool的Callbacks");
  this->set_callback(
    REQ_InsertVariable::MSG_TYPE,
    [this](libmsg::zmqmsg_iter __begin,
           libmsg::zmqmsg_iter __end) -> libmsg::callback_returns {
      logger->trace("Callback <REQUEST INSERT_VARIABLE>");

      auto  __recv_head = __begin->data<msg_head>();
      auto  __recv_body = (__begin + 1)->data<REQ_InsertVariable>();
      void* __recv_buff;
      std::shared_ptr<libmem::base_segment> __seg;
      // pre-check if var name unique
      std::lock_guard<std::mutex> __lock(Py_BasePool::mtx_);
      auto                        __existing_record =
        this->variable_table_.find(__recv_body->var_name);
      if (__existing_record != this->variable_table_.end()) {
        logger->error("新增的变量名({}) 已经存在!", __recv_body->var_name);
        this->reply_fail(
          __recv_head->from, __recv_head->msg_type, "变量名已经存在");
        return callback_returns::fail;
      }
      // check if variable size <= cache bin eps
      if (__recv_body->size <= this->config_.cache_bin_eps) {
        if (__begin + 2 == __end) {
          logger->error("变量大小符合Cache Bin的要求，但并未收到变量的数据!");
          this->reply_fail(
            __recv_head->from,
            __recv_head->msg_type,
            "变量大小符合Cache Bin的要求，但并未收到变量的数据!");
          return callback_returns::fail;
        }
        __recv_buff = (__begin + 2)->data();
        if (__recv_buff == nullptr) {
          logger->error("无法获取变量的数据！");
          this->reply_fail(
            __recv_head->from, __recv_head->msg_type, "无法获取变量的数据！");
          return callback_returns::fail;
        }
        __seg = this->cachbin_STORE(__recv_body->size, __recv_buff);
      } else if (__recv_body->size <= this->config_.instant_bin_eps) {
        __seg = this->statbin_ALLOC(__recv_body->size);
      } else {
        __seg = this->instbin_ALLOC(__recv_body->size);
      }
      // check if allocate fail
      if (__seg == nullptr) {
        logger->error("申请内存失败! Required Size: {} bytes",
                      __recv_body->size);
        this->reply_fail(
          __recv_head->from, __recv_head->msg_type, "申请内存失败");
        return callback_returns::fail;
      }
      // create variable desc
      auto __var_desc =
        std::make_shared<variable_desc>(__recv_head, __recv_body, __seg);
      // insert
      auto __insert_rv = this->variable_table_.insert(
        std::make_pair(__recv_body->var_name, __var_desc));
      auto __insert_pair          = __insert_rv.first;
      __insert_pair->second->name = __insert_pair->first;

      // create response
      msg_head __send_head(
        this->id_, __recv_head->to, RESP_InsertVariable::MSG_TYPE, 1);
      RESP_InsertVariable __send_body;
      __send_body.segment = __seg->to_segmentdesc();
      if (__send_body.segment.segment_type ==
          libmem::SEG_TYPE::cachbin_segment) {
        __send_body.actual_access = ACCESS_TYPE::BY_COPY;
      } else {
        __send_body.actual_access = __recv_body->desire_access;
      }
      this->send(__recv_head->from,
                 { zmq::buffer(&__send_head, sizeof(msg_head)),
                   zmq::buffer(&__send_body, sizeof(RESP_InsertVariable)) });

      logger->trace("Callback <REQUEST INSERT_VARIABLE> Success!");
      return callback_returns::success;
    });

  this->set_callback(
    REQ_GetVariable::MSG_TYPE,
    [this](libmsg::zmqmsg_iter __begin,
           libmsg::zmqmsg_iter __end) -> callback_returns {
      msg_head*        __recv_head = __begin->data<msg_head>();
      REQ_GetVariable* __recv_body = (__begin + 1)->data<REQ_GetVariable>();
      logger->trace("Callback <REQUEST GET_VARIABLE>");
      std::lock_guard<std::mutex> __lock(Py_BasePool::mtx_);
      // try to find the variable
      auto __iter = this->variable_table_.find(__recv_body->var_name);
      if (__iter == this->variable_table_.end()) {
        logger->error("没有找到变量 {}", __recv_body->var_name);
        return callback_returns::fail;
      }
      // 找到了变量
      msg_head __send_head(
        this->id_, __recv_head->from, RESP_GetVariable::MSG_TYPE, 1);
      RESP_GetVariable __send_body(__iter->second);
      void*            __send_buff;
      // if Cache segment
      if (__send_body.segment.segment_type ==
          libmem::SEG_TYPE::cachbin_segment) {
        // retrive cache segment buffer
        __send_buff = this->cachbin_RETRIEVE(__iter->second->segment->id);
        if (__send_buff == nullptr) {
          this->reply_fail(__recv_head->from,
                           REQ_GetVariable::MSG_TYPE,
                           "无法找到Cache Segment的数据");
          return callback_returns::fail;
        }
        __send_head.payload_count = 2;
        this->send(__recv_head->from,
                   { zmq::buffer(&__send_head, sizeof(msg_head)),
                     zmq::buffer(&__send_body, sizeof(RESP_GetVariable)),
                     zmq::buffer(__send_buff, __iter->second->segment->size) });
        logger->trace("Callback <REQUEST GET_VARIABLE> Success!");
        return callback_returns::success;
      }
      // other kinds of segment
      this->send(__recv_head->from,
                 { zmq::buffer(&__send_head, sizeof(msg_head)),
                   zmq::buffer(&__send_body, sizeof(RESP_GetVariable)) });

      logger->trace("Callback <REQUEST GET_VARIABLE> Success!");
      return callback_returns::success;
    });

  this->set_callback(REQ_SetVariable::MSG_TYPE,
                     [this](libmsg::zmqmsg_iter __begin,
                            libmsg::zmqmsg_iter __end) -> callback_returns {
                       // TODO:
                     });

  logger->trace("服务端Pool的Callbacks初始化完毕!");
}

Py_Server::Py_Server(std::string_view name)
  : Py_BasePool(name)
  , shm_kernel::message_handler::base_server()
  , libmem::mmgr({ this->pool_name_,
                   config::CacheBinEps,
                   config::InstantBinEps,
                   config::BatchBinSize,
                   config::BatchBinCount },
                 this->logger)
{
  spdlog::trace("正在初始化共享内存服务端...");
  this->__LOGGER__ = this->logger;
  this->init_META();
  this->pool_status_ = POOL_STATUS::OK;
  logger->trace("共享内存服务端初始化完毕!");
}

Py_Server::~Py_Server()
{
  this->pool_status_ = POOL_STATUS::TERMINATE;
  this->meta_ptr_->ref_count -= 1;

  // TODO: Close all clients
  // this->Py_CloseClient();
}

void
Py_Server::Py_CloseClient(const uint32_t client_id,
                          const bool     terminate_sock,
                          const bool     detach_batch,
                          const bool     detach_instant,
                          const bool     detach_meta)
{
  logger->trace("正在关闭客户端Pool {}", client_id);
  msg_head   __send_head(this->id(), client_id, REQ_Detach::MSG_TYPE, 1);
  REQ_Detach __send_body;
  __send_body.batch   = detach_batch;
  __send_body.instant = detach_instant;
  __send_body.meta    = detach_meta;
  this->send(client_id,
             { zmq::buffer(&__send_head, sizeof(msg_head)),
               zmq::buffer(&__send_body, sizeof(REQ_Detach)) });
  if (terminate_sock) {
    std::this_thread::sleep_for(50ms);
    this->stop_client(client_id);
  }
  logger->trace("客户端Pool {}关闭成功!", client_id);
}

size_t
Py_Server::Py_CacheBinEps() const noexcept
{
  return this->config_.cache_bin_eps;
}
size_t
Py_Server::Py_InstantBinEps() const noexcept
{
  return this->config_.instant_bin_eps;
}

std::string_view
Py_Server::Py_Name() const noexcept
{
  return this->pool_name_;
}
uint32_t
Py_Server::Py_Id() const noexcept
{
  return this->id_;
}
uint32_t
Py_Server::Py_RefCount() const noexcept
{
  return this->meta_ptr_->ref_count;
}
POOL_STATUS
Py_Server::Py_Status() const noexcept
{
  return this->pool_status_;
}
pid_t
Py_Server::Py_OwnerPid() const noexcept
{
  return this->meta_ptr_->owner_pid;
}
const std::list<uint32_t>&
Py_Server::Py_ClientIds() const noexcept
{
  return this->connected_clients;
}
} // namespace shmpy
