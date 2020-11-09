#include "libshmpy.hpp"
#include "data_server.hpp"
#include "data_client.hpp"
#include "var.hpp"

int main()
{
    {
        auto ds = Server("server1", 128);
        std::cout << ds.get_name() << std::endl;
        std::cout << ds.get_ref_count() << std::endl;

        auto dc = Client("server1");
        std::cout << dc.get_name() << std::endl;
        std::cout << dc.get_ref_count() << std::endl;
    }

    std::cout << "Done\n";
}