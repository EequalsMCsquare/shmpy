#include "py_server.hpp"
#include "py_basepool.hpp"
#include "py_config.hpp"
#include "py_error_code.hpp"
#include "py_except.hpp"
#include "py_message.hpp"
#include "py_utils.hpp"
#include "py_variable.hpp"
#include <algorithm>
#include <bits/stdint-uintn.h>
#include <cstring>
#include <endpoint/base_handler.hpp>
#include <endpoint/base_server.hpp>
#include <memory>
#include <mmgr.hpp>
#include <mutex>
#include <optional>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <segment.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <zmq.hpp>

namespace shmpy {

uint32_t
Py_Server::id() const noexcept
{
  return this->_M_Id;
}

void
Py_Server::init_META()
{
  logger_->trace("正在初始化服务端Pool Meta...");
  std::error_code ec;
  this->meta_handle_ =
    std::make_shared<libshm::shm_handle>(MAKE_META_HANDLE_NAME(this->Py_Name()), sizeof(META));
  // map pool meta
  this->meta_ptr_ = reinterpret_cast<META*>(this->meta_handle_->map(ec));
  // check if success
  if (ec) {
    logger_->critical("无法初始化Py_Server的内存池Meta! ({}) {}", ec.value(), ec.message());
    throw std::runtime_error("fail to initialize pool meta!");
  }
  // configure pool meta
  this->meta_ptr_->zmq_recv_port = msgsvr_->recv_port();
  this->meta_ptr_->zmq_send_port = msgsvr_->send_port();
  this->meta_ptr_->ref_count     = 1;
  this->meta_ptr_->owner_pid     = getpid();
  logger_->trace("服务端Pool Meta初始化完毕!");
}

void
Py_Server::reply_fail(const uint32_t to, const int req_type, std::string_view why)
{
  msg_head     __send_head(this->id(), to, RESP_Failure::MSG_TYPE, 1);
  RESP_Failure __send_body(why);
  msgsvr_->send(to,
                { zmq::buffer(&__send_head, sizeof(msg_head)),
                  zmq::buffer(&__send_body, sizeof(RESP_Failure)) });
}

void
Py_Server::init_CALLBACKS()
{
  logger_->trace("正在初始化服务端Pool的Callbacks");
  msgsvr_->set_callback(
    REQ_InsertVariable::MSG_TYPE,
    [this](libmsg::zmqmsg_iter __begin, libmsg::zmqmsg_iter __end) -> libmsg::callback_returns {
      logger_->trace("Callback <REQUEST INSERT_VARIABLE>");

      // auto                                  __recv_head = __begin->data<msg_head>();
      // auto                                  __recv_body = (__begin +
      // 1)->data<REQ_InsertVariable>(); void*                                 __recv_buff;
      // std::shared_ptr<libmem::base_segment> __seg;
      // // pre-check if var name unique
      // std::lock_guard<std::mutex> __lock(Py_BasePool::mtx_);
      // auto __existing_record = this->variable_table_.find(__recv_body->var_name);
      // if (__existing_record != this->variable_table_.end()) {
      //   logger_->error("新增的变量名({}) 已经存在!", __recv_body->var_name);
      //   this->reply_fail(__recv_head->from, __recv_head->msg_type, "变量名已经存在");
      //   return callback_returns::fail;
      // }
      // // check if variable size <= cache bin eps
      // if (__recv_body->size <= mmgr_->config().cache_bin_eps) {
      //   if (__begin + 2 == __end) {
      //     logger_->error("变量大小符合Cache Bin的要求，但并未收到变量的数据!");
      //     this->reply_fail(__recv_head->from,
      //                      __recv_head->msg_type,
      //                      "变量大小符合Cache Bin的要求，但并未收到变量的数据!");
      //     return callback_returns::fail;
      //   }
      //   __recv_buff = (__begin + 2)->data();
      //   if (__recv_buff == nullptr) {
      //     logger_->error("无法获取变量的数据！");
      //     this->reply_fail(__recv_head->from, __recv_head->msg_type, "无法获取变量的数据！");
      //     return callback_returns::fail;
      //   }
      //   __seg = mmgr_->cachbin_STORE(__recv_buff, __recv_body->size);
      // } else if (__recv_body->size <= mmgr_->config().instant_bin_eps) {
      //   __seg = mmgr_->statbin_ALLOC(__recv_body->size);
      // } else {
      //   __seg = mmgr_->instbin_ALLOC(__recv_body->size);
      // }
      // // check if allocate fail
      // if (__seg == nullptr) {
      //   logger_->error("申请内存失败! Required Size: {} bytes", __recv_body->size);
      //   this->reply_fail(__recv_head->from, __recv_head->msg_type, "申请内存失败");
      //   return callback_returns::fail;
      // }
      // // create variable desc
      // auto __var_desc = std::make_shared<variable_desc>(__recv_head, __recv_body, __seg);
      // // insert
      // auto __insert_rv =
      //   this->variable_table_.insert(std::make_pair(__recv_body->var_name, __var_desc));
      // auto __insert_pair          = __insert_rv.first;
      // __insert_pair->second->name = __insert_pair->first;

      // // create response
      // msg_head __send_head(this->id(), __recv_head->to, RESP_InsertVariable::MSG_TYPE, 1);
      // RESP_InsertVariable __send_body;
      // __send_body.segment = __seg->to_segmentdesc();
      // if (__send_body.segment.segment_type == libmem::SEG_TYPE::cachbin_segment) {
      //   __send_body.actual_access = ACCESS_TYPE::BY_COPY;
      // } else {
      //   __send_body.actual_access = __recv_body->desire_access;
      // }
      // msgsvr_->send(__recv_head->from,
      //               { zmq::buffer(&__send_head, sizeof(msg_head)),
      //                 zmq::buffer(&__send_body, sizeof(RESP_InsertVariable)) });

      logger_->trace("Callback <REQUEST INSERT_VARIABLE> Success!");
      return callback_returns::success;
    });

  msgsvr_->set_callback(
    REQ_GetVariable::MSG_TYPE,
    [this](libmsg::zmqmsg_iter __begin, libmsg::zmqmsg_iter __end) -> callback_returns {
      msg_head*        __recv_head = __begin->data<msg_head>();
      REQ_GetVariable* __recv_body = (__begin + 1)->data<REQ_GetVariable>();
      logger_->trace("Callback <REQUEST GET_VARIABLE>");
      std::lock_guard<std::mutex> __lock(Py_BasePool::mtx_);
      // try to find the variable
      auto __iter = this->variable_table_.find(__recv_body->var_name);
      if (__iter == this->variable_table_.end()) {
        logger_->error("没有找到变量 {}", __recv_body->var_name);
        return callback_returns::fail;
      }
      // 找到了变量
      msg_head         __send_head(this->id(), __recv_head->from, RESP_GetVariable::MSG_TYPE, 1);
      RESP_GetVariable __send_body(__iter->second);
      void*            __send_buff;
      // if Cache segment
      if (__send_body.segment.segment_type == libmem::SEG_TYPE::cachbin_segment) {
        // retrive cache segment buffer
        __send_buff = mmgr_->cachbin_RETRIEVE(__iter->second->segment->id);
        if (__send_buff == nullptr) {
          this->reply_fail(
            __recv_head->from, REQ_GetVariable::MSG_TYPE, "无法找到Cache Segment的数据");
          return callback_returns::fail;
        }
        __send_head.payload_count = 2;
        msgsvr_->send(__recv_head->from,
                      { zmq::buffer(&__send_head, sizeof(msg_head)),
                        zmq::buffer(&__send_body, sizeof(RESP_GetVariable)),
                        zmq::buffer(__send_buff, __iter->second->segment->size) });
        logger_->trace("Callback <REQUEST GET_VARIABLE> Success!");
        return callback_returns::success;
      }
      // other kinds of segment
      msgsvr_->send(__recv_head->from,
                    { zmq::buffer(&__send_head, sizeof(msg_head)),
                      zmq::buffer(&__send_body, sizeof(RESP_GetVariable)) });

      logger_->trace("Callback <REQUEST GET_VARIABLE> Success!");
      return callback_returns::success;
    });

  msgsvr_->set_callback(
    REQ_SetVariable::MSG_TYPE,
    [this](libmsg::zmqmsg_iter __begin, libmsg::zmqmsg_iter __end) -> callback_returns {
      // TODO:
    });

  logger_->trace("服务端Pool的Callbacks初始化完毕!");
}

Py_Server::Py_Server(std::string_view name)
  : Py_BasePool(name)
{
  this->_M_Id   = 0;
  this->logger_ = spdlog::stdout_color_mt(fmt::format("{}/server", name));
  logger_->trace("正在初始化共享内存服务端...");
  this->mmgr_ = std::make_unique<libmem::mmgr>(
    this->pool_name_, config::BatchBinSize, config::BatchBinCount, this->logger_);

  this->msgsvr_ = std::make_unique<libmsg::base_server>(this->logger_);
  this->init_META();
  this->pool_status_ = POOL_STATUS::OK;
  logger_->trace("共享内存服务端初始化完毕!");
}

Py_Server::~Py_Server()
{
  this->pool_status_ = POOL_STATUS::TERMINATE;
  this->meta_ptr_->ref_count -= 1;

  std::lock_guard<std::mutex> __lock(Py_BasePool::mtx_);
  if (msgsvr_->client_count() > 0) {
    logger_->trace("正在关闭客户端Pool...");
    // 防止迭代器失效
    std::vector<uint32_t> __tmp_vec(msgsvr_->client_ids().begin(), msgsvr_->client_ids().end());
    for (const auto id : __tmp_vec) {
      this->Py_CloseClient(id, true);
    }
    logger_->trace("客户端Pool关闭完毕!");
  }

  spdlog::drop(this->logger_->name());
}

void
Py_Server::Py_CloseClient(const uint32_t client_id,
                          const bool     terminate_sock,
                          const bool     detach_batch,
                          const bool     detach_instant,
                          const bool     detach_meta)
{
  logger_->trace("正在关闭客户端Pool {}", client_id);
  msg_head   __send_head(this->id(), client_id, REQ_Detach::MSG_TYPE, 1);
  REQ_Detach __send_body;
  __send_body.batch   = detach_batch;
  __send_body.instant = detach_instant;
  __send_body.meta    = detach_meta;
  msgsvr_->send(
    client_id,
    { zmq::buffer(&__send_head, sizeof(msg_head)), zmq::buffer(&__send_body, sizeof(REQ_Detach)) });
  if (terminate_sock) {
    std::this_thread::sleep_for(50ms);
    msgsvr_->stop_client(client_id);
  }
  logger_->trace("客户端Pool {}关闭成功!", client_id);
}

template<typename T>
std::enable_if_t<std::is_fundamental_v<T>, void>
Py_Server::HANDLE_FundamentalTypeCacheInsert(std::string_view name,
                                             const T&         var,
                                             const uint32_t   inserter_id,
                                             std::error_code& ec) noexcept

{
  DTYPE __dtype;
  if constexpr (std::is_same_v<T, long>) {
    __dtype = DTYPE::INT;
  } else if (std::is_same_v<T, double>) {
    __dtype = DTYPE::FLOAT;
  } else if (std::is_same_v<T, bool>) {
    __dtype = DTYPE::BOOL;
  }
  // lock
  std::lock_guard<std::mutex> __lock(this->mtx_);
  auto                        __iter = this->variable_table_.find(name);
  if (__iter != this->variable_table_.end()) {
    logger_->error("变量 ({}) 已经存在!", name);
    ec = ShmpyErrc::VariableExist;
    return;
  }
  auto __seg = mmgr_->cachbin_STORE(
    sizeof(T), [&var](void* buffer) { *(static_cast<T*>(buffer)) = var; }, ec);
  if (ec && __seg == nullptr) {
    logger_->error("变量({})拷贝数据失败! ({}) {}", name, ec.value(), ec.message());
    return;
  }
  logger_->trace("变量({})拷贝数据成功", name);
  auto __insert_rv = this->variable_table_.insert(
    std::make_pair(std::string(name.begin(), name.end()),
                   std::make_shared<variable_desc>(__dtype, sizeof(T), false, inserter_id, __seg)));
  auto __insert_pair          = __insert_rv.first;
  __insert_pair->second->name = __insert_pair->first;
  logger_->trace("变量({})insert成功！", name);
  return;
}

template<typename T>
std::enable_if_t<std::is_fundamental_v<T>, void>
Py_Server::HANDLE_FundamentalTypeCacheSet(std::string_view name,
                                          const T&         var,
                                          const uint32_t   setter_id,
                                          std::error_code& ec) noexcept
{
  DTYPE __dtype;
  if constexpr (std::is_same_v<T, long>) {
    __dtype = DTYPE::INT;
  } else if (std::is_same_v<T, double>) {
    __dtype = DTYPE::FLOAT;
  } else if (std::is_same_v<T, bool>) {
    __dtype = DTYPE::BOOL;
  }
  // find the var
  std::lock_guard<std::mutex> __lock(this->mtx_);

  auto __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    logger_->error("没有找到变量({})!", name);
    ec = ShmpyErrc::VariableNotFound;
    return;
  }

  __iter->second->add_attached_client(setter_id);
  int rv = mmgr_->cachbin_SET(
    __iter->second->segment->id,
    sizeof(T),
    [var](void* buffer) { *(static_cast<T*>(buffer)) = var; },
    ec);
  if (ec) {
    logger_->error("变量({})修改失败!", name);
  } else {
    logger_->trace("变量({})修改成功!", name);
    __iter->second->dtype = __dtype;
    __iter->second->size  = sizeof(T);
  }
}

void
Py_Server::HANDLE_GenericTypeCacheDel(std::string_view name,
                                      const bool       force,
                                      const bool       notify_attachers)
{
  mtx_.lock();
  auto __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    logger_->error("没有找到变量 ({})", name);
    mtx_.unlock();
    return;
  }
  if (force) {
    int rv = this->mmgr_->cachbin_DEALLOC(__iter->second->segment->id);
    if (rv == -1) {
      logger_->error("删除变量 ({})失败!", name);
      mtx_.unlock();
      return;
    }
    auto __tmp = __iter->second;
    this->variable_table_.erase(__iter);
    mtx_.unlock();
    if (notify_attachers) {
      // TODO:
    }
  } else {
    // TODO: check ref_count
    if (__iter->second->ref_count == 1) {
      int rv = this->mmgr_->cachbin_DEALLOC(__iter->second->segment->id);
      if (rv == -1) {
        logger_->error("删除变量 ({})失败!", name);
        mtx_.unlock();
        return;
      }
      this->variable_table_.erase(__iter);
      mtx_.unlock();
      return;
    }
  }
}

template<typename T>
std::optional<T>
Py_Server::HANDLE_GenericCacheGet(std::string_view name, const uint32_t requester_id)
{
  // try to find
  auto __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    logger_->error("没有找到变量 ({})", name);
    return std::nullopt;
  }
  __iter->second->add_attached_client(requester_id);
  void* __buff = mmgr_->cachbin_RETRIEVE(__iter->second->segment->id);
  if (__buff) {
    return *(static_cast<T*>(__buff));
  } else {
    logger_->error("无法获取变量({})的buffer!", name);
    return std::nullopt;
  }
}

template<>
std::optional<std::string_view>
Py_Server::HANDLE_GenericCacheGet(std::string_view name, const uint32_t requester_id)
{
  // try to find
  auto __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    logger_->error("没有找到变量 ({})", name);
    return std::nullopt;
  }
  __iter->second->add_attached_client(requester_id);
  Py_String* __py_str =
    static_cast<Py_String*>(mmgr_->cachbin_RETRIEVE(__iter->second->segment->id));
  if (__py_str) {
    return std::string_view(__py_str->data(), __py_str->size_);
  } else {
    logger_->error("无法获取变量({})的buffer!", name);
    return std::nullopt;
  }
}

template<>
void
Py_Server::HANDLE_ComplexTypeCacheInsert(std::string_view       name,
                                         const std::string_view var,
                                         const uint32_t         inserter_id,
                                         std::error_code&       ec) noexcept
{
  ec.clear();
  size_t __req_size = var.size() + sizeof(size_t);

  std::lock_guard<std::mutex> __lock(this->mtx_);
  auto                        __iter = this->variable_table_.find(name);
  if (__iter != this->variable_table_.end()) {
    logger_->error("变量 ({}) 已经存在!", name);
    ec = ShmpyErrc::VariableExist;
    return;
  }
  auto __seg = mmgr_->cachbin_STORE(
    __req_size,
    [&var](void* buffer) {
      size_t* __strsz = static_cast<size_t*>(buffer);
      *__strsz        = var.size();
      std::memcpy(__strsz + 1, var.data(), var.size());
    },
    ec);
  if (ec && __seg == nullptr) {
    logger_->error("变量({})拷贝数据失败! ({}) {}", name, ec.value(), ec.message());
    return;
  }
  logger_->trace("变量({})拷贝数据成功", name);
  auto __insert_rv            = this->variable_table_.insert(std::make_pair(
    std::string(name.begin(), name.end()),
    std::make_shared<variable_desc>(DTYPE::PY_STRING, __req_size, false, inserter_id, __seg)));
  auto __insert_pair          = __insert_rv.first;
  __insert_pair->second->name = __insert_pair->first;
  logger_->trace("变量({})insert成功！", name);
  return;
}

template<>
void
Py_Server::HANDLE_ComplexTypeCacheSet(std::string_view       name,
                                      const std::string_view var,
                                      const uint32_t         setter_id,
                                      std::error_code&       ec) noexcept
{
  size_t __req_size = sizeof(size_t) + var.size();
  // find the var
  std::lock_guard<std::mutex> __lock(this->mtx_);

  auto __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    logger_->error("没有找到变量({})!", name);
    ec = ShmpyErrc::VariableNotFound;
    return;
  }

  __iter->second->add_attached_client(setter_id);
  int rv = mmgr_->cachbin_SET(
    __iter->second->segment->id,
    __req_size,
    [var](void* buffer) {
      size_t* __strsz = static_cast<size_t*>(buffer);
      *__strsz        = var.size();
      std::memcpy(__strsz + 1, var.data(), var.size());
    },
    ec);
  if (ec) {
    logger_->error("变量({})修改失败!", name);
  } else {
    logger_->trace("变量({})修改成功!", name);
    __iter->second->dtype = DTYPE::PY_STRING;
    __iter->second->size  = __req_size;
  }
}

// Insert
void
Py_Server::Py_IntInsert(std::string_view name, const py::int_& number)
{
  std::error_code ec;
  int64_t         __num = number.cast<int64_t>();
  this->HANDLE_FundamentalTypeCacheInsert(name, __num, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}
void
Py_Server::Py_FloatInsert(std::string_view name, const py::float_& number)
{
  std::error_code ec;
  double          __num = number.cast<double>();
  this->HANDLE_FundamentalTypeCacheInsert(name, __num, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}
void
Py_Server::Py_BoolInsert(std::string_view name, const py::bool_& boolean)
{
  std::error_code ec;
  auto            __bool = boolean.cast<bool>();
  this->HANDLE_FundamentalTypeCacheInsert(name, __bool, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}
void
Py_Server::Py_StrInsert(std::string_view name, std::string_view str)
{
  std::error_code ec;
  this->HANDLE_ComplexTypeCacheInsert(name, str, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}

// Get
py::int_
Py_Server::Py_IntGet(std::string_view name)
{
  auto __num = this->HANDLE_GenericCacheGet<long>(name, this->_M_Id);
  if (!__num) {
    throw ShmpyExcept(ShmpyErrc::VariableNotFound);
  }
  return __num.value();
}
py::float_
Py_Server::Py_FloatGet(std::string_view name)
{
  auto __num = this->HANDLE_GenericCacheGet<double>(name, this->_M_Id);
  if (!__num) {
    throw ShmpyExcept(ShmpyErrc::VariableNotFound);
  }
  return __num.value();
}
py::bool_
Py_Server::Py_BoolGet(std::string_view name)
{
  auto __bool = this->HANDLE_GenericCacheGet<bool>(name, this->_M_Id);
  if (!__bool) {
    throw ShmpyExcept(ShmpyErrc::VariableNotFound);
  }
  return __bool.value();
}
py::str
Py_Server::Py_StrGet(std::string_view name)
{
  auto __str = this->HANDLE_GenericCacheGet<std::string_view>(name, this->_M_Id);
  if (!__str) {
    throw ShmpyExcept(ShmpyErrc::VariableNotFound);
  }
  return py::str(__str.value().data(), __str.value().size());
}

// Set
void
Py_Server::Py_IntSet(std::string_view name, const py::int_& number)
{
  std::error_code ec;
  int64_t         __num = number.cast<int64_t>();
  this->HANDLE_FundamentalTypeCacheSet(name, __num, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}
void
Py_Server::Py_FloatSet(std::string_view name, const py::float_& number)
{
  std::error_code ec;
  auto            __num = number.cast<double>();
  this->HANDLE_FundamentalTypeCacheSet(name, __num, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}
void
Py_Server::Py_BoolSet(std::string_view name, const py::bool_& boolean)
{
  std::error_code ec;
  auto            __bool = boolean.cast<bool>();
  this->HANDLE_FundamentalTypeCacheSet(name, __bool, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}
void
Py_Server::Py_StrSet(std::string_view name, std::string_view str)
{
  std::error_code ec;
  this->HANDLE_ComplexTypeCacheSet(name, str, this->_M_Id, ec);
  if (ec) {
    throw ShmpyExcept(ec);
  }
}

void
Py_Server::Py_GenericCacheVarDel(std::string_view name,
                                 const bool       force,
                                 const bool       notify_attachers)
{
  this->HANDLE_GenericTypeCacheDel(name, force, notify_attachers);
}

// high-level functions

void
Py_Server::Py_GenericInsert(std::string_view name, const py::object& obj)
{
  if (py::isinstance<py::int_>(obj)) {
    this->Py_IntInsert(name, obj);
  } else if (py::isinstance<py::float_>(obj)) {
    this->Py_FloatInsert(name, obj);
  } else if (py::isinstance<py::bool_>(obj)) {
    this->Py_BoolInsert(name, obj);
  } else if (py::isinstance<py::str>(obj)) {
    this->Py_StrInsert(name, obj.cast<std::string_view>());
  }
}

void
Py_Server::Py_GenericDelete(std::string_view name)
{
  this->HANDLE_GenericTypeCacheDel(name, false, true);
}

void
Py_Server::Py_GenericSet(std::string_view name, py::object& obj)
{
  auto __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    throw ShmpyExcept(ShmpyErrc::VariableNotFound);
  }
  switch (__iter->second->dtype) {
    case DTYPE::INT:
      this->Py_IntSet(name, obj);
    case DTYPE::FLOAT:
      this->Py_FloatSet(name, obj);
    case DTYPE::BOOL:
      this->Py_BoolSet(name, obj);
    case DTYPE::PY_STRING:
      this->Py_StrSet(name, obj.cast<std::string>());
  }
}

py::object
Py_Server::Py_GenericGet(std::string_view name)
{
  auto __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    throw ShmpyExcept(ShmpyErrc::VariableNotFound);
  }
  switch (__iter->second->dtype) {
    case DTYPE::INT:
      return this->Py_IntGet(name);
    case DTYPE::FLOAT:
      return this->Py_FloatGet(name);
    case DTYPE::BOOL:
      return this->Py_BoolGet(name);
    case DTYPE::PY_STRING:
      return this->Py_StrGet(name);
  }
}

// properties
size_t
Py_Server::Py_InstantBinEps() const noexcept
{
  return this->instant_bin_eps_;
}

std::string_view
Py_Server::Py_Name() const noexcept
{
  return this->pool_name_;
}
uint32_t
Py_Server::Py_Id() const noexcept
{
  return msgsvr_->id();
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
  return msgsvr_->client_ids();
}
} // namespace shmpy
