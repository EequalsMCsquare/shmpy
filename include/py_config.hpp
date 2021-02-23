#pragma once

#include "py_common.hpp"
#include <regex>

namespace shmpy::config {

inline static auto TRACE_RX    = std::regex{ "trace", std::regex_constants::icase };
inline static auto DEBUG_RX    = std::regex{ "debug", std::regex_constants::icase };
inline static auto INFO_RX     = std::regex{ "info", std::regex_constants::icase };
inline static auto WARN_RX     = std::regex{ "warn", std::regex_constants::icase };
inline static auto ERROR_RX    = std::regex{ "error", std::regex_constants::icase };
inline static auto CRITICAL_RX = std::regex{ "critical", std::regex_constants::icase };

///
/// \brief When memory required size is less than CacheBinEps, buffer will be
/// stored in cache bin
///
static size_t CacheBinEps = 1_KB;

// batch 43154.0 KB

static std::vector<size_t> BatchBinSize{
  8,     16,    24,    32,    48,    64,    80,    96,     128,    144,    160,    176,   192,
  224,   240,   256,   272,   304,   368,   400,   432,    496,    512,    576,    608,   672,
  736,   768,   800,   864,   928,   960,   992,   1_KB,   2_KB,   4_KB,   8_KB,   12_KB, 16_KB,
  24_KB, 32_KB, 40_KB, 48_KB, 64_KB, 80_KB, 96_KB, 128_KB, 144_KB, 176_KB, 192_KB, 256_KB
};

static std::vector<size_t> BatchBinCount{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                                          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                                          64, 64, 64, 64, 64, 64, 64, 64, 32, 32, 32, 32, 32,
                                          32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 };

static size_t InstantBinEps = BatchBinSize.back() * 4;

void
Py_SetCacheEps(const size_t size) noexcept;
const size_t&
Py_CacheEps() noexcept;

void
Py_SetInstantEps(const size_t size) noexcept;
const size_t&
Py_InstantEps() noexcept;
void
Py_SetBatchConfig(const std::vector<size_t>& bin_size, const std::vector<size_t>& bin_count);

const std::vector<size_t>&
Py_BatchBinSize() noexcept;
const std::vector<size_t>&
Py_BatchBinCount() noexcept;

void
Py_SetLogLevel(spdlog::level::level_enum level);
}