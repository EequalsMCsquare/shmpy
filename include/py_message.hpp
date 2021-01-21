#pragma once

#include "py_dtype.hpp"
#include "py_variable.hpp"

#include <msg/base_msg.hpp>
#include <segment.hpp>

namespace shmpy {

struct REQ_InsertVariable
{
  inline static int MSG_TYPE = 1;
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
  inline static int MSG_TYPE = 2;
  shm_kernel::memory_manager::segmentdesc segment;
  ACCESS_TYPE actual_access;
};

struct RESP_Failure
{
  inline static int MSG_TYPE = 3;
  char message[256];

  RESP_Failure(std::string_view message)
  {
    if (message.size() >= 256) {
      // message incomplete copy.
    }
    std::strncpy(this->message, message.data(), 256);
  }

  RESP_Failure(const char* message)
  {
    if (std::strlen(message) >= 256) {
      // message incomplete copy
    }
    std::strncpy(this->message, message, 256);
  }
  RESP_Failure(std::string&& message)
  {
    if (message.size() >= 256) {
      // message incomplete copy
    }
    std::strncpy(this->message, message.c_str(), 256);
  }
};

}
