#pragma once

namespace shmpy::literal::mem {

    constexpr std::size_t
    operator""_B(unsigned long long __nbytes)
    {return __nbytes;}

    constexpr std::size_t
    operator"" _KB(unsigned long long __nkbytes)
    {return __nkbytes * 1024;}

    constexpr std::size_t
    operator"" _MB(unsigned long long __nmbytes)
    {return __nmbytes * 1024 * 1024;}

    constexpr std::size_t
    operator"" _GB(unsigned long long __ngbytes)
    {return __ngbytes * 1024 * 1024 * 1024;}

}