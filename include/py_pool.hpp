#pragma once

#include "py_common.hpp"

namespace shmpy
{

enum class POOL_STATUS
{
  OK,
  DETACH,
  TERMINATE,
};

struct POOL_META
{
  pid_t owner_pid;
  uint16_t msgsvr_rep_port;
  uint16_t msgsvr_pub_port;
  std::atomic_uint32_t client_counter;
  std::atomic_uint32_t ref_count;
};
}