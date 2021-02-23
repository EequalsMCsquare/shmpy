#pragma once

#include <ipc/shmhdl.hpp>
#include <memory_manager/mem_literals.hpp>
#include <memory_manager/mmgr.hpp>
#include <memory_manager/segment.hpp>
#include <memory_manager/smgr.hpp>
#include <message_handler/client.hpp>
#include <message_handler/message.hpp>
#include <message_handler/server.hpp>
#include <spdlog/spdlog.h>
#include <system_error>
#include <vector>

namespace shmpy {
using shm_t             = ipc::shmhdl;
using msgclt_t          = shm_kernel::message_handler::client;
using msgsvr_t          = shm_kernel::message_handler::server;
using msghead_t         = shm_kernel::message_handler::message_header;
using mmgr_t            = shm_kernel::memory_manager::mmgr;
using smgr_t            = shm_kernel::memory_manager::smgr;
using base_segment_t    = shm_kernel::memory_manager::base_segment;
using cache_segment_t   = shm_kernel::memory_manager::cache_segment;
using static_segment_t  = shm_kernel::memory_manager::static_segment;
using instant_segment_t = shm_kernel::memory_manager::instant_segment;
using segment_info_t    = shm_kernel::memory_manager::segment_info;

template<typename T>
using sptr = std::shared_ptr<T>;
template<typename T>
using uptr     = std::unique_ptr<T>;
using logger_t = spdlog::logger;

}
