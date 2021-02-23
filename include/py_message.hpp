#pragma once

#include "py_common.hpp"
#include "py_ec.hpp"

namespace shmpy {

enum class DTYPE;
class variable_desc;

enum class Py_Commands
{
  detach_pool = 1,
};

struct RESP_Failure
{
  inline static uint32_t MSG_TYPE = 1;
  ShmpyErrc              ec;
};

template<int __MSG_TYPE__>
struct __REQ_VARIABLE_GENERIC_INSERT__
{
  inline static int MSG_TYPE = __MSG_TYPE__;
  char              var_name[256];
  size_t            size;
  DTYPE             dtype;
};

using REQ_VariableCacheInsert   = __REQ_VARIABLE_GENERIC_INSERT__<20>;
using REQ_VariableStaticInsert  = __REQ_VARIABLE_GENERIC_INSERT__<21>;
using REQ_VariableInstantInsert = __REQ_VARIABLE_GENERIC_INSERT__<22>;

struct RESP_VariableCacheInsert
{
  inline static int MSG_TYPE = 23;

  bool success;

  char msg[128];
};

struct RESP_VariableShmInsert
{
  inline static int MSG_TYPE = 24;
  ShmpyErrc         ec;
  segment_info_t    segment;
  DTYPE             dtype;
  bool              is_bp;
  size_t            size;

  RESP_VariableShmInsert(sptr<variable_desc> var_desc);
  RESP_VariableShmInsert(ShmpyErrc ec);
  RESP_VariableShmInsert(sptr<base_segment_t> segment,
                         const DTYPE          dtype,
                         const bool           is_bp,
                         const size_t         size);
};
}