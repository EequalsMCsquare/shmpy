#pragma once

#include "py_dtype.hpp"

#include <memory>
#include <msg/base_msg.hpp>
#include <segment.hpp>

namespace shmpy {
namespace libmem = ::shm_kernel::memory_manager;

enum class ACCESS_TYPE;
class variable_desc;
struct base_variable;

struct RESP_Failure
{
  inline static int MSG_TYPE = 1;
  char              message[256];

  RESP_Failure(std::string_view message);
  RESP_Failure(const char* message);
};

struct REQ_DetachPool
{
  inline static int MSG_TYPE = 2;
  bool              meta;
  bool              batch;
  bool              instant;
};

template<int msg_type>
struct REQ_VariableGenericInsert
{
  inline static int MSG_TYPE = msg_type;

  char var_name[256];

  size_t size;

  DTYPE dtype;
};

using REQ_VariableCacheInsert   = REQ_VariableGenericInsert<20>;
using REQ_VariableStaticInsert  = REQ_VariableGenericInsert<21>;
using REQ_VariableInstantInsert = REQ_VariableGenericInsert<22>;

struct RESP_VariableCacheInsert
{
  inline static int MSG_TYPE = 23;

  bool success;

  char msg[128];
};

struct RESP_VariableShmInsert
{
  inline static int MSG_TYPE = 24;
  bool              success;
  union
  {
    char msg[128];
    struct
    {
      libmem::segmentdesc segment;
      DTYPE               dtype;
      ACCESS_TYPE         access_type;
      bool                is_bp;
      size_t              size;
    };
  };

  RESP_VariableShmInsert(std::shared_ptr<variable_desc> var_desc);
  RESP_VariableShmInsert(std::string_view __error);
  RESP_VariableShmInsert(std::shared_ptr<libmem::base_segment> segment,
                         const DTYPE                           dtype,
                         const ACCESS_TYPE                     access_type,
                         const bool                            is_bp,
                         const size_t                          size);
};

} // namespace shmpy
