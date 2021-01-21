#include <batch.hpp>
#include <bins/cache_bin.hpp>
#include <bins/instant_bin.hpp>
#include <endpoint/base_server.hpp>

#include "py_basepool.hpp"
#include "py_config.hpp"
#include "py_message.hpp"

namespace shmpy {
class Py_Server
  : public libmsg::base_server
  , public Py_BasePool
{
  using callback_returns = shm_kernel::message_handler::callback_returns;
  using msg_head = shm_kernel::message_handler::msg_head;

private:
  std::atomic_size_t segment_counter_{ 0 };
  const size_t cache_bin_eps_;
  const size_t instant_bin_eps_;

  void init_CALLBACKS() override
  {
    logger->trace("正在初始化服务端Pool的Callbacks");
    this->set_callback(
      REQ_InsertVariable::MSG_TYPE,
      [this](libmsg::zmqmsg_iter __begin,
             libmsg::zmqmsg_iter __end) -> libmsg::callback_returns {
        logger->trace("Callback <REQUEST INSERT_VARIABLE>");

        libmsg::msg_head* __recv_head = __begin->data<libmsg::msg_head>();
        REQ_InsertVariable* __recv_req =
          (__begin + 1)->data<REQ_InsertVariable>();

        // check if variable name exist in allocated_variables_
        std::lock_guard<std::mutex> __lock(this->mtx_);
        if (auto __iter = this->allocated_variables_.find(__recv_req->var_name);
            __iter == this->allocated_variables_.end()) {
          if (__recv_req->size <= config::CacheBinEps) {
            // allocate with cache_bin_

            // check if variable buffer is send
            if (__begin + 2 == __end) {
              constexpr std::string_view __WHY =
                "变量的大小符合CacheBin的要求, 但是并没有收到数据";
              logger->error(__WHY);
              libmsg::msg_head __resp_head(
                this->id(), __recv_head->from, RESP_Failure::MSG_TYPE, 1);
              RESP_Failure __resp_body(__WHY);

              this->send(__recv_head->from,
                         { zmq::buffer(&__resp_head, sizeof(libmsg::msg_head)),
                           zmq::buffer(&__resp_body, sizeof(RESP_Failure)) });
              return callback_returns::fail;
            }

            void* __var_buffer = (__begin + 2)->data();
            // check not null
            if (__var_buffer == nullptr) {
              constexpr std::string_view __WHY = "无法获取数据";
              logger->error(__WHY);
              libmsg::msg_head __resp_head(
                this->id(), __recv_head->from, RESP_Failure::MSG_TYPE, 1);
              RESP_Failure __resp_body(__WHY);
              this->send(__recv_head->from,
                         { zmq::buffer(&__resp_head, sizeof(libmsg::msg_head)),
                           zmq::buffer(&__resp_body, sizeof(RESP_Failure)) });
              return callback_returns::fail;
            }
            // set respond message
            RESP_InsertVariable __resp;
            auto& __seg = __resp.segment;
            __seg.segment_id =
              this->cache_bin_->store(__var_buffer, __recv_req->size);
            __seg.seg_type_ = libmem::SEG_TYPE::cachbin_segment;
            __resp.actual_access = ACCESS_TYPE::BY_COPY;
            std::strncpy(__seg.arena_name, this->name().data(), 128);

            // store the variable info
            auto __insert_rv = this->allocated_variables_.insert(std::make_pair(
              __recv_req->var_name,
              std::make_shared<variable_desc>(__recv_head, __recv_req, __seg)));
            auto __insert_iter = __insert_rv.first;
            __insert_iter->second->name = __insert_iter->first;

            // make respond msg head
            libmsg::msg_head __resp_head(
              this->id(), __recv_head->from, RESP_InsertVariable::MSG_TYPE, 1);
            __resp_head.need_reply = false;

            // send response
            this->send(__recv_head->from,
                       { zmq::buffer(&__resp_head, sizeof(libmsg::msg_head)),
                         zmq::buffer(&__resp, sizeof(RESP_InsertVariable)) });
            return callback_returns::success;

          } else if (__recv_req->size <= config::InstantBinEps) {
            // allocate with static bin batch
            std::shared_ptr<libmem::static_segment> __seg;
            for (const auto& b : this->batches_) {
              __seg = b->allocate(__recv_req->size);
              if (__seg) {
                // if allocate success, break loop
                break;
              }
            }
            // all of batches can't meet the requirement, add a new batch
            if (!__seg) {
              // if fail, add a new batch
              auto& __new_batch = this->add_BATCH();
              // allocate using the new batch
              __seg = __new_batch->allocate(__recv_req->size);
            }
            // if still fail
            if (__seg == nullptr) {
              constexpr std::string_view __WHY = "无法分配空间, 真奇怪...";
              logger->error(__WHY);
              // make respond msg head
              libmsg::msg_head __resp_head(
                this->id(), __recv_head->from, RESP_Failure::MSG_TYPE, 1);
              __resp_head.need_reply = false;
              // make response
              RESP_Failure __resp_body(__WHY);
              // send response
              this->send(__recv_head->from,
                         { zmq::buffer(&__resp_head, sizeof(libmsg::msg_head)),
                           zmq::buffer(&__resp_body, sizeof(RESP_Failure)) });
              return callback_returns::fail;
            }
            // if success

            // make response body
            RESP_InsertVariable __resp_body;
            __resp_body.segment = libmem::segmentdesc(*__seg.get());
            __resp_body.actual_access = __recv_req->desire_access;

            // make response head
            libmsg::msg_head __resp_head(
              this->id(), __recv_head->from, RESP_InsertVariable::MSG_TYPE, 1);
            __resp_head.need_reply = false;
            // store varialbe info
            auto __insert_rv = this->allocated_variables_.insert(std::make_pair(
              __recv_req->var_name,
              std::make_shared<variable_desc>(__recv_head, __recv_req, __seg)));
            // set variable_desc name
            auto __insert_iter = __insert_rv.first;
            __insert_iter->second->name = __insert_iter->first;
            // reply
            this->send(
              __recv_head->from,
              { zmq::buffer(&__resp_head, sizeof(libmsg::msg_head)),
                zmq::buffer(&__resp_body, sizeof(RESP_InsertVariable)) });
            return callback_returns::success;

          } else {
            // allocate with instant_bin_
          }

        } else {
          constexpr std::string_view __WHY = "变量的名字已经存在.";
          logger->error(__WHY);
          msg_head __resp_head(
            this->id(), __recv_head->from, RESP_Failure::MSG_TYPE, 1);
          RESP_Failure __resp_body(__WHY);
          this->send(__recv_head->from,
                     { zmq::buffer(&__resp_head, sizeof(msg_head)),
                       zmq::buffer(&__resp_body, sizeof(RESP_Failure)) });
          return callback_returns::fail;
        }

        logger->trace("Callback <REQUEST INSERT_VARIABLE>");
      });
    logger->trace("服务端Pool的Callbacks初始化完毕!");
  }

  void init_META()
  {
    this->logger->trace("正在初始化服务端Pool Meta...");
    std::error_code ec;
    this->meta_handle_ = std::make_shared<libshm::shm_handle>(
      std::move(std::string(this->name().begin(), this->name().end())),
      sizeof(META));
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
    this->meta_ptr_->ref_count = 1;
    this->meta_ptr_->owner_pid = getpid();
    this->logger->trace("服务端Pool Meta初始化完毕!");
  }

  std::unique_ptr<libmem::batch>& add_BATCH()
  {
    logger->trace("正在添加一组Static Bin...");
    this->batches_.push_back(
      std::make_unique<libmem::batch>(this->pool_name_,
                                      this->batches_.size(),
                                      config::BatchBinSize,
                                      config::BatchBinCount));
    logger->trace("一组Static Bin添加完毕!");
    return this->batches_.back();
  }

  void init_CACHE_BIN()
  {
    logger->trace("正在初始化Cache Bin...");
    this->cache_bin_ = std::make_unique<libmem::cache_bin>(
      this->segment_counter_, this->pool_name_, config::CacheBinEps);
    logger->trace("Cache Bin初始化完毕!");
  }

  void init_INSTANT_BIN()
  {
    logger->trace("正在初始化Instant Bin...");
    this->instant_bin_ = std::make_unique<libmem::instant_bin>(
      this->segment_counter_, this->pool_name_);
    logger->trace("Instant Bin初始化完毕!");
  }

protected:
  std::map<std::string, std::shared_ptr<variable_desc>, std::less<>>
    allocated_variables_;
  std::vector<std::unique_ptr<libmem::batch>> batches_;
  std::unique_ptr<libmem::cache_bin> cache_bin_;
  std::unique_ptr<libmem::instant_bin> instant_bin_;

  void PyInt_SET(std::string_view name, const py::int_& number) override {}

  void PyFloat_SET(std::string_view name, const py::float_& number) override {}

public:
  Py_Server(const std::string& name)
    : shm_kernel::message_handler::base_server()
    , Py_BasePool(name, this->logger)
    , cache_bin_eps_(config::CacheBinEps)
    , instant_bin_eps_(config::InstantBinEps)
  {
    logger->trace("正在初始化共享内存服务端...");
    this->batches_.reserve(4);
    this->init_META();
    this->init_CACHE_BIN();
    this->init_INSTANT_BIN();
    this->add_BATCH();
    logger->trace("共享内存服务端初始化完毕!");
  }

  const size_t Py_CacheBinEps() const noexcept { return this->cache_bin_eps_; }
  const size_t Py_InstantBinEps() const noexcept
  {
    return this->instant_bin_eps_;
  }
};
} // namespace shmpy
