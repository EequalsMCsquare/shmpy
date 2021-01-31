#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string_view>

#include "py_basepool.hpp"
#include "py_client.hpp"
#include "py_dtype.hpp"
#include "py_server.hpp"
#include "py_variable.hpp"

namespace py = pybind11;

namespace shmpy {

PYBIND11_MODULE(shmpy, m)
{

  py::enum_<POOL_STATUS>(m, "POOL_STATUS")
    .value("ok", POOL_STATUS::OK)
    .value("detach", POOL_STATUS::DETACH)
    .value("terminate", POOL_STATUS::TERMINATE)
    .export_values();

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
    .def("str_get", &Py_Server::Py_StrGet, py::arg("var_name"))

    .def("Del",
         &shmpy::Py_Server::Py_GenericCacheVarDel,
         py::arg("var_name"),
         py::arg("force_delete")     = false,
         py::arg("notify_attachers") = false)
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