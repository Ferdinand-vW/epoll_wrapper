#include "epoll_wrapper/Error.h"
#include <bitset>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>

namespace epoll_wrapper
{
std::ostream &operator<<(std::ostream &os, const ErrorCode &ec)
{
    return os << std::to_string(ec);
}

std::string errorMaskToString(const ErrorCodeMask &ecm)
{
    std::stringstream ss;

    bool first = true;
    auto toOut = [&ss, ecm, &first](const auto err) {
        if (ecm & err)
        {
            if (first)
            {
                ss << err;
                first = false;
            }
            else
            {
                ss << ", " << err;
            }
        }
    };

    toOut(ErrorCode::EbadF);
    toOut(ErrorCode::Eexist);
    toOut(ErrorCode::Efault);
    toOut(ErrorCode::Eintr);
    toOut(ErrorCode::Einval);
    toOut(ErrorCode::Eloop);
    toOut(ErrorCode::EmFile);
    toOut(ErrorCode::EnFile);
    toOut(ErrorCode::EnoEnt);
    toOut(ErrorCode::EnoMem);
    toOut(ErrorCode::EnoSpc);
    toOut(ErrorCode::Eperm);

    return ss.str();
}

ErrorCode fromEpollError(u_int16_t err)
{
    switch (err)
    {
    case 0:
        return ErrorCode::None;
    case EBADF:
        return ErrorCode::EbadF;
    case EEXIST:
        return ErrorCode::Eexist;
    case EINVAL:
        return ErrorCode::Einval;
    case ELOOP:
        return ErrorCode::Eloop;
    case ENOENT:
        return ErrorCode::EnoEnt;
    case ENOMEM:
        return ErrorCode::EnoMem;
    case ENOSPC:
        return ErrorCode::EnoSpc;
    case EPERM:
        return ErrorCode::Eperm;
    case EFAULT:
        return ErrorCode::Efault;
    case EINTR:
        return ErrorCode::Eintr;
    case EMFILE:
        return ErrorCode::EmFile;
    case ENFILE:
        return ErrorCode::EnFile;
    default:
        return ErrorCode::Unknown;
    }
}

ErrorCodeMask operator|(ErrorCodeMask mask, ErrorCode err)
{
    return mask | static_cast<ErrorCodeMask>(err);
}

ErrorCodeMask operator|(ErrorCode err1, ErrorCode err2)
{
    return static_cast<ErrorCodeMask>(err1) | err2;
}

ErrorCodeMask operator&(ErrorCodeMask mask, ErrorCode err)
{
    return mask & static_cast<ErrorCodeMask>(err);
}
} // namespace epoll_wrapper

namespace std
{
std::string to_string(epoll_wrapper::ErrorCode c)
{
    using namespace epoll_wrapper;
    switch (c)
    {
    case ErrorCode::None:
        return "";
        break;
    case ErrorCode::Unknown:
        return "UNKNOWN";
        break;
    case ErrorCode::EbadF:
        return "EBADF";
        break;
    case ErrorCode::Eexist:
        return "EEXISTS";
        break;
    case ErrorCode::Einval:
        return "EINVAL";
        break;
    case ErrorCode::Eloop:
        return "ELOOP";
        break;
    case ErrorCode::EnoEnt:
        return "ENOENT";
        break;
    case ErrorCode::EnoMem:
        return "ENOMEM";
        break;
    case ErrorCode::EnoSpc:
        return "ENOSPEC";
        break;
    case ErrorCode::Eperm:
        return "EPERM";
        break;
    case ErrorCode::Efault:
        return "EFAULT";
        break;
    case ErrorCode::Eintr:
        return "EINTR";
        break;
    case ErrorCode::EmFile:
        return "EMFILE";
        break;
    case ErrorCode::EnFile:
        return "ENFILE";
        break;
    }
}
} // namespace std