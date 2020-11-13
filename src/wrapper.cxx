#include "libshmpy.hpp"
#include "data_server.hpp"
#include "data_client.hpp"
#include "var.hpp"
#include <pybind11/iostream.h>

PYBIND11_MODULE(shmpy, m) {

    py::class_<Server>(m, "Server")
        .def(py::init<const std::string&, const std::uint32_t&, const std::uint32_t&>(),
            py::arg("name"), py::arg("capacity"), py::arg("max_attac_proc") = 128)
        .def("insert", &Server::insert, py::arg("name"), py::arg("data"))
        .def("set", &Server::set, py::arg("name"), py::arg("data"))
        .def("get", &Server::get, py::arg("name"))
        .def("del", &Server::del, py::arg("name"))
        .def_property_readonly("name", &Server::get_name)
        .def_property_readonly("capacity", &Server::get_capacity)
        .def_property_readonly("size", &Server::get_size)
        .def_property_readonly("ref_count", &Server::get_ref_count)
        .def_property_readonly("owner_pid", &Server::get_owner_pid)
        .def_property_readonly("max_clients", &Server::get_max_clients)
        .def_property_readonly("status", &Server::Pyget_status)
        .def_property_readonly("client_ids", &Server::get_client_ids);

    py::class_<Client>(m, "Client")
        .def(py::init<const std::string&>(), py::arg("name"))
        .def("insert", &Client::insert, py::arg("name"), py::arg("data"))
        .def("set", &Client::set, py::arg("name"), py::arg("data"))
        .def("get", &Client::get, py::arg("name"))
        .def("del", &Client::del, py::arg("name"))
        .def_property_readonly("id",&Client::get_id)
        .def_property_readonly("name", &Client::get_name)
        .def_property_readonly("capacity", &Client::get_capacity)
        .def_property_readonly("size", &Client::get_size)
        .def_property_readonly("ref_count", &Client::get_ref_count)
        .def_property_readonly("owner_pid", &Client::get_owner_pid)
        .def_property_readonly("max_clients", &Client::get_max_clients)
        .def_property_readonly("status", &Client::Pyget_status);

    py::register_exception<pool_error>(m, "PoolError");
    py::register_exception<msgq_error>(m, "MsgqError");
    py::register_exception<shm_error>(m, "ShmError");

    m.def("test", []() {
        // interesting. 当使用memoryview 构建np.ndarray的时候返回的才不是copy!
        double *buffer = new double[12];
        for(int i = 0; i < 12; i++)
            buffer[i] = i;
        std::vector<ssize_t> shape{3,4};
        std::vector<ssize_t> strides{32,8};
        py::scoped_ostream_redirect stream(
                std::cout,                               // std::ostream&
                py::module_::import("sys").attr("stdout") // Python output
        );
        std::cout << buffer << std::endl;
        return py::array(py::memoryview::from_buffer(buffer, 8, "d", shape, strides));
    });
    m.def("test2", [](const py::object& obj) {
        py::scoped_estream_redirect stream (
            std::cout, py::module_::import("sys").attr("stdout")
        );
        if (py::isinstance(obj,py::module_::import("numpy").attr("ndarray"))) {
            std::cout << "is numpy array" << std::endl;
        }
    });
}