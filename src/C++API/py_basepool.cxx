#include "py_basepool.hpp"
#include "py_message.hpp"

#include <bits/stdint-uintn.h>
#include <msg/base_msg.hpp>
#include <regex>

#include <fmt/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace shmpy {
std::string
Py_BasePool::MAKE_META_HANDLE_NAME(std::string_view name)
{
  return fmt::format("shmpy#{}", name);
}

Py_BasePool::Py_BasePool(std::string_view pool_name)
{
  this->pool_name_ = pool_name;
}

void
Py_BasePool::set_log_level(const std::string& level)
{
  if (std::regex_match(level, TRACE_RX)) {
    spdlog::apply_all(
      [](const auto __logger) { __logger->set_level(spdlog::level::debug); });
  } else if (std::regex_match(level, DEBUG_RX)) {
    spdlog::apply_all(
      [](const auto __logger) { __logger->set_level(spdlog::level::trace); });
  } else if (std::regex_match(level, INFO_RX)) {
    spdlog::apply_all(
      [](const auto __logger) { __logger->set_level(spdlog::level::info); });
  } else if (std::regex_match(level, WARN_RX)) {
    spdlog::apply_all(
      [](const auto __logger) { __logger->set_level(spdlog::level::warn); });
  } else if (std::regex_match(level, ERROR_RX)) {
    spdlog::apply_all(
      [](const auto __logger) { __logger->set_level(spdlog::level::err); });
  } else if (std::regex_match(level, CRITICAL_RX)) {
    spdlog::apply_all([](const auto __logger) {
      __logger->set_level(spdlog::level::critical);
    });
  } else {
    spdlog::warn("unknown log level, use default level -> error");
    spdlog::apply_all(
      [](const auto __logger) { __logger->set_level(spdlog::level::err); });
  }
}

// void Py_BasePool::insert(const std::string_view name, const py::object &var)
// {
//   // Try to find the name in attached_variables_
//   auto __iter = this->attched_variables_.find(name);
//   if (__iter != this->attched_variables_.end()) {
//     __LOGGER__->error("变量的名称必须是唯一的.");
//     throw std::runtime_error(
//         "variable name already exist in local's attached_variables. If you "
//         "want to modify the variable, please use set(...) instead.");
//   }
//   // Type checking
// }

} // namespace shmpy
