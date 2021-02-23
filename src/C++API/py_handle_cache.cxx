#include "py_dtype.hpp"
#include "py_server.hpp"
#include <mutex>

namespace shmpy {


template<DTYPE dtype>
void
Py_Server::HANDLE_CacheInsert(std::string_view             name,
                              const size_t                 size,
                              const uint32_t               inserter_id,
                              std::function<void(void*)>&& callback,
                              std::error_code&             ec) noexcept
{
  ec.clear();
  // lock
  std::lock_guard<std::mutex> __lock(this->var_mtx_);
  auto                        __iter = this->variable_table_.find(name);
  if (__iter == this->variable_table_.end()) {
    auto __seg = mmgr_->CACHE_STORE(size, std::move(callback), ec);
    if (ec && __seg == nullptr) {
      logger_->error("fail to copy data of variable '{}'. ({}) {}", name, ec.value(), ec.message());
      return;
    }
    logger_->trace("variable '{}' copy success!", name);
    auto __insert_rv = this->variable_table_.insert(
      std::make_pair(name, std::make_shared<variable_desc>(dtype, size, false, __seg)));
    // setup variable_desc
    auto __insert_pair           = __insert_rv.first;
    __insert_pair->second->name_ = __insert_pair->first;
    __insert_pair->second->make_client_attached(inserter_id);
    logger_->trace("variable '{}' insert success!", name);
  } else {
    logger_->error("variable ({}) already exist!", name);
    ec = ShmpyErrc::VariableExist;
    return;
  }
}

template<DTYPE dtype>
void
Py_Server::HANDLE_CacheSet(std::string_view             name,
                           const size_t                 size,
                           const uint32_t               setter_id,
                           std::function<void(void*)>&& callback,
                           std::error_code&             ec) noexcept
{
  ec.clear();
  // lock
  std::map<std::string, sptr<variable_desc>, std::less<>>::iterator __iter;
  {
    std::lock_guard __lock(this->var_mtx_);
    __iter = this->variable_table_.find(name);
  }
  if (__iter == this->variable_table_.end()) {
    // if not found call CacheInsert
    this->HANDLE_CacheInsert<dtype>(name, size, setter_id, std::move(callback), ec);
    return;
  } else {
    __iter->second->make_client_attached(setter_id);
    mmgr_->CACHE_SET(__iter->second->segment_->id, size, std::move(callback), ec);
    if (ec) {
      logger_->error("fail to set variable '{}'. ({}) {}", name, ec.value(), ec.message());
      return;
    } else {
      logger_->trace("variable '{}' set success!", name);
      // update variable desc
      __iter->second->dtype_ = dtype;
      __iter->second->size_  = size;
      return;
    }
  }
}

void
Py_Server::HANDLE_CacheDelete(std::string_view name, std::error_code& ec) noexcept
{
  ec.clear();
  std::lock_guard __lock(this->var_mtx_);
  auto            __iter = this->variable_table_.find(name);
  if (__iter != this->variable_table_.end()) {
    this->mmgr_->CACHE_DEALLOC(__iter->second->segment_->id, ec);
    if (ec) {
      logger_->error("unable to dealloc variable '{}' buffer. {}({}) {}",
                     name,
                     ec.category().name(),
                     ec.value(),
                     ec.message());
      return;
    } else {
      this->variable_table_.erase(__iter);
      return;
    }
  }
  logger_->error("unable to find variable '{}'", name);
  ec = ShmpyErrc::VariableNotFound;
}

template<typename T>
std::optional<T>
Py_Server::HANDLE_CacheGet(std::string_view name,
                           const uint32_t   getter_id,
                           std::error_code& ec) noexcept
{
  ec.clear();
  // lock
  std::lock_guard __lock(this->var_mtx_);
  auto            __iter = this->variable_table_.find(name);
  if (__iter != this->variable_table_.end()) {
    __iter->second->make_client_attached(getter_id);
    void* __buff = mmgr_->CACHE_RETRIEVE(__iter->second->segment_->id, ec);
    if (__buff) {
      return *(static_cast<T*>(__buff));
    } else {
      logger_->error(
        "unable to retrieve variable '{}''s buffer. ({}) {}", name, ec.value(), ec.message());
      ec = ShmpyErrc::UnableToGetVariableBuffer;
      return std::nullopt;
    }
  }
  logger_->error("unable to find variable '{}'", name);
  ec = ShmpyErrc::VariableNotFound;
  return std::nullopt;
}

template<>
std::optional<std::string_view>
Py_Server::HANDLE_CacheGet(std::string_view name,
                           const uint32_t   getter_id,
                           std::error_code& ec) noexcept
{
  ec.clear();
  std::lock_guard __lock(this->var_mtx_);

  auto __iter = this->variable_table_.find(name);
  if (__iter != this->variable_table_.end()) {
    __iter->second->make_client_attached(getter_id);
    void* __buff = mmgr_->CACHE_RETRIEVE(__iter->second->segment_->id, ec);
    if (!ec) {
      Py_String* __py_str = static_cast<Py_String*>(__buff);
      return std::string_view{ __py_str->data(), __py_str->size_ };
    } else {
      logger_->error(
        "unable to retrieve variable '{}''s buffer. ({}) {}", name, ec.value(), ec.message());
      ec = ShmpyErrc::UnableToGetVariableBuffer;
      return std::nullopt;
    }
  }
  logger_->error("unable to find variable '{}'", name);
  ec = ShmpyErrc::VariableNotFound;
  return std::nullopt;
}

// template<typename T>
// void
// Py_Server::HANDLE_CacheInsert(std::string_view name,
//                               const T&         data,
//                               const size_t     size,
//                               const uint32_t   inserter_id,
//                               std::error_code& ec) noexcept
// {
//   ec.clear();
//   DTYPE dtype;
//   if constexpr (std::is_same_v<T, int32_t>) {
//     dtype = DTYPE::int32;
//   } else if constexpr (std::is_same_v<T, int64_t>) {
//     dtype = DTYPE::int64;
//   } else if constexpr (std::is_same_v<T, uint32_t>) {
//     dtype = DTYPE::uint32;
//   } else if constexpr (std::is_same_v<T, uint64_t>) {
//     dtype = DTYPE::uint64;
//   } else if constexpr (std::is_same_v<T, float>) {
//     dtype = DTYPE::float32;
//   } else if constexpr (std::is_same_v<T, double>) {
//     dtype = DTYPE::float64;
//   } else if constexpr (std::is_same_v<T, bool>) {
//     dtype = DTYPE::boolean;
//   }
//   // lock
//   std::lock_guard<std::mutex> __lock(this->var_mtx_);
//   auto                        __iter = this->variable_table_.find(name);
//   if (__iter == this->variable_table_.end()) {
//     auto __seg = mmgr_->CACHE_STORE(
//       size, [&data, &size](void* buffer) { std::memcpy(buffer, data, size); }, ec);
//     if (ec && __seg == nullptr) {
//       logger_->error("fail to copy data of variable '{}'. ({}) {}", name, ec.value(),
//       ec.message()); return;
//     }
//     logger_->trace("variable '{}' copy success!", name);
//     auto __insert_rv = this->variable_table_.insert(
//       std::make_pair(name, std::make_shared<variable_desc>(dtype, size, false, __seg)));
//     // setup variable_desc
//     auto __insert_pair           = __insert_rv.first;
//     __insert_pair->second->name_ = __insert_pair->first;
//     __insert_pair->second->make_client_attached(inserter_id);
//     logger_->trace("variable '{}' insert success!", name);
//   } else {
//     logger_->error("variable ({}) already exist!", name);
//     ec = ShmpyErrc::VariableExist;
//     return;
//   }
// }

}