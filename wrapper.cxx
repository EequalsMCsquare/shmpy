#include "libshmpy.hpp"
#include "data_server.hpp"
#include "data_client.hpp"
#include "var.hpp"

PYBIND11_MODULE(shmpy, m) {
    py::class_<Server>(m, "Server")
        .def(py::init<const std::string&, const std::uint32_t&, const std::uint32_t&>(),
            py::arg("name"), py::arg("capacity"), py::arg("max_attac_proc") = 128)
        .def("insert", &Server::insert, py::arg("name"), py::arg("data"))
        .def("set", &Server::set, py::arg("name"), py::arg("data"))
        .def("del", &Server::del, py::arg("name"))
        .def_property_readonly("name", &Server::get_name)
        .def_property_readonly("capacity", &Server::get_capacity)
        .def_property_readonly("size", &Server::get_size)
        .def_property_readonly("ref_count", &Server::get_ref_count)
        .def_property_readonly("owner_pid", &Server::get_owner_pid)
        .def_property_readonly("max_clients", &Server::get_max_clients)
        .def_property_readonly("client_ids", &Server::get_client_ids);

    py::class_<Client>(m, "Client")
        .def(py::init<const std::string&>(), py::arg("name"))
        .def("insert", &Server::insert, py::arg("name"), py::arg("data"))
        .def("set", &Server::set, py::arg("name"), py::arg("data"))
        .def("del", &Server::del, py::arg("name"))
        .def_property_readonly("name", &Server::get_name)
        .def_property_readonly("capacity", &Server::get_capacity)
        .def_property_readonly("size", &Server::get_size)
        .def_property_readonly("ref_count", &Server::get_ref_count)
        .def_property_readonly("owner_pid", &Server::get_owner_pid)
        .def_property_readonly("max_clients", &Server::get_max_clients)
        .def_property_readonly("client_ids", &Server::get_client_ids);
}