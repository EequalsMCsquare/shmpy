#include "libshmpy.hpp"
#include "data_server.hpp"
#include "data_client.hpp"
#include <pybind11/embed.h>
#include <pybind11/pytypes.h>
#include <gtest/gtest.h>
#include <var.hpp>

namespace py = pybind11;
using namespace std::string_literals;

class TestCase_Var : public testing::Test {

public:
    std::shared_ptr<Server> s;
    std::shared_ptr<Client> c;

    virtual void SetUp() {
        this->s = std::make_shared<Server>("server2", 128);
        this->c = std::make_shared<Client>("server2");
    }

    virtual void TearDown() {
        s.reset();
        c.reset();
    }
};

struct Entity {
public:
    std::string name;
    Entity(const std::string& _name):name(_name) {}
    ~Entity() {
        std::cout << "Calling " << this->name << " ~Entity()" << std::endl;
    }
};

TEST_F(TestCase_Var, CreateNdArray) {
    py::scoped_interpreter g{};
    auto randn = py::module_::import("numpy").attr("random").attr("randn");
    auto src = randn(3,4);
    s->insert("arr", src );
    ASSERT_EQ(s->get_size(), 1);
    c->get("arr");

}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}