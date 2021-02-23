#pragma once

#include "py_common.hpp"
#include "py_dtype.hpp"
#include "py_pool.hpp"
#include "py_variable.hpp"
#include <type_traits>

namespace shmpy {

struct POOL_META;

class Py_Server
{

#ifndef NDEBUG
public:
#else
private:
#endif
  std::string name_;
  POOL_STATUS pool_status_;
  sptr<shm_t> meta_shmhdl_;
  POOL_META*  meta_;
  std::mutex  mtx_;
  // TODO: attached variables
  std::map<std::string, sptr<shm_t>, std::less<>> attached_shms_;
  sptr<logger_t>                                  logger_;

  uptr<msgsvr_t> msgsvr_;
  uptr<mmgr_t>   mmgr_;

  std::map<std::string, sptr<variable_desc>, std::less<>> variable_table_;
  std::mutex                                              var_mtx_;

  void     init_CALLBACKS();
  void     init_META();
  uint32_t id() const noexcept;

  // template<typename T>
  // void HANDLE_CacheInsert(std::string_view name,
  //                         const T&         data,
  //                         const size_t     size,
  //                         const uint32_t   inserter_id,
  //                         std::error_code& ec) noexcept;

  template<typename T>
  std::enable_if_t<std::is_base_of_v<Py_Dtype, T>, void> HANDLE_CacheInsert(
    std::string_view name,
    const uint32_t   inserter_id,
    std::error_code& ec) noexcept;

  template<DTYPE dtype>
  void HANDLE_CacheInsert(std::string_view             name,
                          const size_t                 size,
                          const uint32_t               inserter_id,
                          std::function<void(void*)>&& callback,
                          std::error_code&             ec) noexcept;

  template<DTYPE dtype>
  void HANDLE_CacheSet(std::string_view             name,
                       const size_t                 size,
                       const uint32_t               setter_id,
                       std::function<void(void*)>&& callback,
                       std::error_code&             ec) noexcept;

  template<typename T>
  std::optional<T> HANDLE_CacheGet(std::string_view name,
                                   const uint32_t   getter_id,
                                   std::error_code& ec) noexcept;

  void HANDLE_CacheDelete(std::string_view name, std::error_code& ec) noexcept;

  // template<DTYPE dtype>
  // void HANDLE_StaticInsert(std::string_view name,
  //                          const size_t     size,
  //                          const uint32_t   inserter_id,
  //                          std::error_code& ec) noexcept;

  // template<DTYPE dtype>
  // void HANDLE_StaticSet(std::string_view name,
  //                       const size_t     size,
  //                       const uint32_t   setter_id,
  //                       std::error_code& ec) noexcept;

  // template<DTYPE dtype>
  // void HANDLE_InstantInsert(std::string_view name,
  //                           const size_t     size,
  //                           const uint32_t   inserter_id,
  //                           std::error_code& ec) noexcept;

  // template<DTYPE dtype>
  // void HANDLE_InstantSet(std::string_view name,
  //                        const size_t     size,
  //                        const uint32_t   setter_id,
  //                        std::error_code& ec);

public:
  Py_Server(std::string_view name);
  ~Py_Server();
  void Py_CloseClient(const uint32_t client_id, std::error_code& ec) noexcept;

  void Py_InsertInt32(std::string_view name, const int32_t number);

  uint32_t         Py_Id() const noexcept;
  std::string_view Py_Name() const noexcept;
  POOL_STATUS      Py_Status() const noexcept;
  uint32_t         Py_RefCount() const noexcept;
};
}