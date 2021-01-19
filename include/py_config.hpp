#pragma once

#include <mem_literals.hpp>
#include <vector>

namespace shmpy::config {
///
/// \brief When memory required size is less than CacheBinEps, buffer will be
/// stored in cache bin
///
size_t CacheBinEps = 1_KB;

std::vector<size_t> BatchBinSize{ 512,   1_KB,  2_KB,  4_KB,  8_KB,
                                  12_KB, 16_KB, 24_KB, 32_KB, 48_KB,
                                  64_KB, 80_KB, 96_KB, 128_KB };

std::vector<size_t> BatchBinCount{ 64, 64, 64, 64, 64, 64, 64,
                                   64, 50, 50, 50, 50, 32, 32 };

size_t InstantBinEps = BatchBinSize.back() * 8;
}
