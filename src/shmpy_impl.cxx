#include <map>
#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <spdlog/common.h>
#include <string>
#include <string_view>

#include "py_basepool.hpp"
#include "py_client.hpp"
#include "py_config.hpp"
#include "py_dtype.hpp"
#include "py_except.hpp"
#include "py_server.hpp"
#include "py_variable.hpp"

namespace shmpy {

PYBIND11_MODULE(shmpy, m)
{
  using LOG_LEVEL = spdlog::level::level_enum;
  namespace py    = pybind11;

  auto config_module = m.def_submodule("config");
  config_module.def("set_cache_eps", &config::Py_SetCacheEps, py::arg("nbytes"));
  config_module.def("cache_eps", &config::Py_CacheEps);
  config_module.def("instant_eps", &config::Py_InstantEps);
  config_module.def("set_instant_eps", &config::Py_SetInstantEps, py::arg("nbytes"));
  config_module.def(
    "set_batch_config", &config::Py_SetBatchConfig, py::arg("bin_size"), py::arg("bin_count"));
  config_module.def("batch_config", []() {
    return std::map<std::string, std::vector<size_t>>{ { "bin size", config::BatchBinSize },
                                                       { "bin count", config::BatchBinCount } };
  });
  config_module.def("set_log_level", config::Py_SetLogLevel, py::arg("level"));

  py::enum_<spdlog::level::level_enum>(config_module, "LOG_LEVEL")
    .value("trace", LOG_LEVEL::trace)
    .value("info", LOG_LEVEL::info)
    .value("debug", LOG_LEVEL::debug)
    .value("warn", LOG_LEVEL::warn)
    .value("error", LOG_LEVEL::err)
    .value("critical", LOG_LEVEL::critical)
    .value("off", LOG_LEVEL::off)
    .export_values();

  py::enum_<POOL_STATUS>(m, "POOL_STATUS")
    .value("ok", POOL_STATUS::OK)
    .value("detach", POOL_STATUS::DETACH)
    .value("terminate", POOL_STATUS::TERMINATE)
    .export_values();

  py::register_exception<ShmpyExcept>(m, "ShmpyExcept");

  py::class_<Py_Server>(m, "server")
    .def(py::init<std::string_view>(), py::arg("name"))

    .def("int_insert", &Py_Server::Py_IntInsert, py::arg("var_name"), py::arg("var"))
    .def("int_set", &Py_Server::Py_IntSet, py::arg("var_name"), py::arg("var"))
    .def("int_get", &Py_Server::Py_IntGet, py::arg("var_name"))

    .def("float_insert", &Py_Server::Py_FloatInsert, py::arg("var_name"), py::arg("var"))
    .def("float_set", &Py_Server::Py_FloatSet, py::arg("var_name"), py::arg("var"))
    .def("float_get", &Py_Server::Py_FloatGet, py::arg("var_name"))

    .def("bool_insert", &Py_Server::Py_BoolInsert, py::arg("var_name"), py::arg("var"))
    .def("bool_set", &Py_Server::Py_BoolSet, py::arg("var_name"), py::arg("var"))
    .def("bool_get", &Py_Server::Py_BoolGet, py::arg("var_name"))

    .def("str_insert", &Py_Server::Py_StrInsert, py::arg("var_name"), py::arg("var"))
    .def("str_set", &Py_Server::Py_StrSet, py::arg("var_name"), py::arg("var"))
    .def("str_get", &Py_Server::Py_StrGet, py::arg("var_name"), py::return_value_policy::move)

    .def("insert", &Py_Server::Py_GenericInsert, py::arg("var_name"), py::arg("variable"))
    .def("get", &Py_Server::Py_GenericGet, py::arg("var_name"), py::return_value_policy::move)
    .def("set", &Py_Server::Py_GenericSet, py::arg("var_name"), py::arg("var"))
    .def("rm", &Py_Server::Py_GenericDelete, py::arg("var_name"))
    .def("delete", &Py_Server::Py_GenericDelete, py::arg("var_name"))

    .def("log_on", &Py_Server::set_log_level, py::arg("log_level"))
    .def("close_client",
         &Py_Server::Py_CloseClient,
         py::arg("client_id"),
         py::arg("stop_socket")    = true,
         py::arg("detach_batch")   = true,
         py::arg("detach_instant") = true,
         py::arg("detach_meta")    = true)
    .def_property_readonly("name", &Py_Server::Py_Name)
    .def_property_readonly("ref_count", &Py_Server::Py_RefCount)
    .def_property_readonly("instant_eps", &Py_Server::Py_InstantBinEps)
    .def_property_readonly("status", &Py_Server::Py_Status)
    .def_property_readonly("id", &Py_Server::Py_Id)
    .def_property_readonly("clients_id", &Py_Server::Py_ClientIds)
    .def_property_readonly("owner_pid", &Py_Server::Py_OwnerPid);

  py::class_<Py_Client>(m, "client")
    .def(py::init<std::string_view>(), py::arg("name"))
    .def("log_on", &Py_Client::set_log_level, py::arg("log_level"))
    .def_property_readonly("name", &Py_Client::Py_Name)
    .def_property_readonly("id", &Py_Client::Py_Id)
    .def_property_readonly("ref_count", &Py_Client::Py_RefCount)
    .def_property_readonly("status", &Py_Client::Py_Status)
    .def_property_readonly("owner_pid", &Py_Client::Py_OwnerPid);
}

}