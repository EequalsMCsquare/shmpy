#pragma once

#include <mem_literals.hpp>
#include <vector>

namespace shmpy::config {
///
/// \brief When memory required size is less than CacheBinEps, buffer will be
/// stored in cache bin
///
static size_t CacheBinEps = 1_KB;

static std::vector<size_t> BatchBinSize{ 512,   1_KB,  2_KB,  4_KB,  8_KB,
                                         12_KB, 16_KB, 24_KB, 32_KB, 48_KB,
                                         64_KB, 80_KB, 96_KB, 128_KB };

static std::vector<size_t> BatchBinCount{ 64, 64, 64, 64, 64, 64, 64,
                                          64, 50, 50, 50, 50, 32, 32 };

static size_t InstantBinEps = BatchBinSize.back() * 8;

static void
set_cache_bin_eps(const size_t size) noexcept;
static void
set_instant_bin_eps(const size_t size) noexcept;
static void
config_batch_bin(const std::vector<size_t>& bin_size,
                 const std::vector<size_t>& bin_count) noexcept;
} // namespace shmpy::config
