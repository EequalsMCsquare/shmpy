#include "libshmpy.hpp"
#include "data_server.hpp"
#include "data_client.hpp"
#include "var.hpp"

int main()
{

    auto ds = new Server("server1", 128);
    auto dc1 = new Client("server1");
    // auto dc2 = new Client("server1");

    std::cout << ds->get_ref_count() << std::endl;
    std::cout << dc1->get_ref_count() << std::endl;
    // std::cout << dc2->get_ref_count() << std::endl;
    
    delete dc1;
    delete ds;
    // delete dc2;

    std::cout << "Done\n";
}