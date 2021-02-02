#include "py_config.hpp"
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace shmpy::config {

void
Py_SetCacheEps(const size_t size) noexcept
{
  CacheBinEps = size;
}
const size_t&
Py_CacheEps() noexcept
{
  return CacheBinEps;
}

void
Py_SetInstantEps(const size_t size) noexcept
{
  InstantBinEps = size;
}
const size_t&
Py_InstantEps() noexcept
{
  return InstantBinEps;
}
void
Py_SetBatchConfig(const std::vector<size_t>& bin_size, const std::vector<size_t>& bin_count)
{
  if (bin_count.size() != bin_size.size()) {
    throw std::invalid_argument("len(bin_size) must == len(bin_count)");
  }
  BatchBinCount = bin_count;
  BatchBinSize  = bin_size;
}

const std::vector<size_t>&
Py_BatchBinSize() noexcept
{
  return BatchBinSize;
}
const std::vector<size_t>&
Py_BatchBinCount() noexcept
{
  return BatchBinCount;
}
void
Py_SetLogLevel(spdlog::level::level_enum level)
{
  spdlog::set_level(level);
}
}