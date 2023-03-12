#pragma once

#include <ostream>
#include <sys/types.h>

namespace epoll_wrapper
{
    using ErrorCodeMask = u_int16_t;
    enum class ErrorCode : ErrorCodeMask
        { None = 0u
        , Unknown = 1u << 0
        , EbadF   = 1u << 1
        , Eexist  = 1u << 2
        , Einval  = 1u << 3
        , Eloop   = 1u << 4
        , EnoEnt  = 1u << 5
        , EnoMem  = 1u << 6
        , EnoSpc  = 1u << 7
        , Eperm   = 1u << 8
        , Efault  = 1u << 9
        , Eintr   = 1u << 10
        , EmFile  = 1u << 11
        , EnFile  = 1u << 12 
        };

    std::ostream& operator<<(std::ostream&, const ErrorCode&);
    std::string errorMaskToString(const ErrorCodeMask& ecm);

    ErrorCodeMask operator|(ErrorCodeMask mask, ErrorCode err);
    ErrorCodeMask operator|(ErrorCode err1, ErrorCode err2);
    ErrorCodeMask operator&(ErrorCodeMask mask, ErrorCode err);

    ErrorCode fromEpollError(u_int16_t errc);
}

namespace std
{
    std::string to_string(epoll_wrapper::ErrorCode c);
}