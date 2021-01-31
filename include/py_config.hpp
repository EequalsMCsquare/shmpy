#pragma once

#include <mem_literals.hpp>
#include <vector>

namespace shmpy::config {
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

static void
set_cache_bin_eps(const size_t size) noexcept;
static void
set_instant_bin_eps(const size_t size) noexcept;
static void
config_batch_bin(const std::vector<size_t>& bin_size,
                 const std::vector<size_t>& bin_count) noexcept;
} // namespace shmpy::config
