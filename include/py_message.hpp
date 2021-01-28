#pragma once

#include "py_dtype.hpp"

#include <memory>
#include <msg/base_msg.hpp>
#include <segment.hpp>

namespace shmpy {

enum class ACCESS_TYPE;
class variable_desc;

struct RESP_Failure
{
  inline static int MSG_TYPE = 1;
  char              message[256];

  RESP_Failure(std::string_view message);
  RESP_Failure(const char* message);
};

struct REQ_InsertVariable
{
  inline static int MSG_TYPE = 2;
  /// Variable Name
  char var_name[256];
  /// size of variable
  size_t size;
  /// if Py_Buffer_Protocol
  bool is_pybuff_protocol;
  /// variable data type
  DTYPE dtype;
  ///
  /// \brief variable desire_access
  /// \details 如果变量储存在了cache bin
  /// 则无法通过引用来获取他，每次get都是从cache bin拷贝这个变量
  ACCESS_TYPE desire_access;
};

struct RESP_InsertVariable
{
  inline static int                       MSG_TYPE = 3;
  shm_kernel::memory_manager::segmentdesc segment;
  ACCESS_TYPE                             actual_access;
};

struct REQ_SetVariable
{
  inline static int MSG_TYPE = 4;

  char var_name[256];

  size_t size;

  bool is_pybuff_protocol;

  DTYPE dtype;

  ACCESS_TYPE desire_access;
};

struct RESP_SetVariable
{
  inline static int MSG_TYPE = 5;

  shm_kernel::memory_manager::segmentdesc segment;

  ACCESS_TYPE actual_access;
};

struct REQ_GetVariable
{
  inline static int MSG_TYPE = 6;
  char              var_name[256];
};

struct RESP_GetVariable
{
  inline static int MSG_TYPE = 7;

  shm_kernel::memory_manager::segmentdesc segment;

  bool is_pybuff_protocol;

  DTYPE dtype;

  ACCESS_TYPE access;

  explicit RESP_GetVariable(std::shared_ptr<variable_desc> var_desc);
};

struct REQ_DelVariable
{
  inline static int MSG_TYPE = 8;

  char var_name[256];

  bool safe_delete = true;
};

struct RESP_DelVariable
{
  inline static int MSG_TYPE = 9;

  size_t ref_count_after_delete;
};

struct REQ_RenameVariable
{
  inline static int MSG_TYPE = 10;

  char origin_name[256];

  char new_name[256];
};

struct RESP_RenameVariable
{
  inline static int MSG_TYPE = 11;
};

struct REQ_Detach
{
  inline static int MSG_TYPE = 12;
  bool              meta;
  bool              batch;
  bool              instant;
};

} // namespace shmpy
