#include <batch.hpp>
#include <bins/cache_bin.hpp>
#include <bins/instant_bin.hpp>
#include <endpoint/base_server.hpp>

#include "py_basepool.hpp"

namespace shmpy {
class Py_Server
  : public libmsg::base_server
  , public Py_BasePool
{

private:
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
  {
    this->init_META();
  }
};
} // namespace shmpy
