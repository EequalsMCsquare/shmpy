#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string_view>

#include "py_basepool.hpp"
#include "py_client.hpp"
#include "py_dtype.hpp"
#include "py_server.hpp"
#include "py_variable.hpp"

namespace py = pybind11;

namespace shmpy {}

PYBIND11_MODULE(shmpy, m)
{

  py::enum_<shmpy::POOL_STATUS>(m, "POOL_STATUS")
    .value("ok", shmpy::POOL_STATUS::OK)
    .value("detach", shmpy::POOL_STATUS::DETACH)
    .value("terminate", shmpy::POOL_STATUS::TERMINATE)
    .export_values();

  py::class_<shmpy::Py_Server>(m, "server")
    .def(py::init<std::string_view>(), py::arg("name"))
    .def("log_on", &shmpy::Py_Server::set_log_level, py::arg("log_level"))
    .def("close_client",
         &shmpy::Py_Server::Py_CloseClient,
         py::arg("client_id"),
         py::arg("stop_socket")    = true,
         py::arg("detach_batch")   = true,
         py::arg("detach_instant") = true,
         py::arg("detach_meta")    = true)
    .def_property_readonly("name", &shmpy::Py_Server::Py_Name)
    .def_property_readonly("ref_count", &shmpy::Py_Server::Py_RefCount)
    .def_property_readonly("instant_eps", &shmpy::Py_Server::Py_InstantBinEps)
    .def_property_readonly("cache_eps", &shmpy::Py_Server::Py_CacheBinEps)
    .def_property_readonly("status", &shmpy::Py_Server::Py_Status)
    .def_property_readonly("id", &shmpy::Py_Server::Py_Id)
    .def_property_readonly("clients_id", &shmpy::Py_Server::Py_ClientIds)
    .def_property_readonly("owner_pid", &shmpy::Py_Server::Py_OwnerPid);

  py::class_<shmpy::Py_Client>(m, "client")
    .def(py::init<std::string_view>(), py::arg("name"))
    .def("log_on", &shmpy::Py_Client::set_log_level, py::arg("log_level"))
    .def_property_readonly("name", &shmpy::Py_Client::Py_Name)
    .def_property_readonly("id", &shmpy::Py_Client::Py_Id)
    .def_property_readonly("ref_count", &shmpy::Py_Client::Py_RefCount)
    .def_property_readonly("status", &shmpy::Py_Client::Py_Status)
    .def_property_readonly("owner_pid", &shmpy::Py_Client::Py_OwnerPid);
}
