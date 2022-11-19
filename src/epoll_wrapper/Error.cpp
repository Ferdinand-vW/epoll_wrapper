#include "epoll_wrapper/Error.h"

namespace epoll_wrapper
{
    std::ostream& operator<<(std::ostream& os, const ErrorCode& ec)
    {
        switch(ec)
        {
        case ErrorCode::None:
            os << "NONE";
            break;
        case ErrorCode::Unknown:
            os << "UNKNOWN";
            break;
        case ErrorCode::EbadF:
            os << "EBADF";
            break;
        case ErrorCode::Eexist:
            os << "EEXISTS";
            break;
        case ErrorCode::Einval:
            os << "EINVAL";
            break;
        case ErrorCode::Eloop:
            os << "ELOOP";
            break;
        case ErrorCode::EnoEnt:
            os << "ENOENT";
            break;
        case ErrorCode::EnoMem:
            os << "ENOMEM";
            break;
        case ErrorCode::EnoSpc:
            os << "ENOSPEC";
            break;
        case ErrorCode::Eperm:
            os << "EPERM";
            break;
        case ErrorCode::Efault:
            os << "EFAULT";
            break;
        case ErrorCode::Eintr:
            os << "EINTR";
            break;
        case ErrorCode::EmFile:
            os << "EMFILE";
            break;
        case ErrorCode::EnFile:
            os << "ENFILE";
            break;
        }

        return os;
    }

    ErrorCodeMask fromEpollError(int errc)
    {
        ErrorCodeMask ec;

        if (errc >= 0)
        {
            return ErrorCodeMask{};
        }

        int err = errno;

        if (err & EBADF)  { ec = ec | ErrorCode::EbadF;  }
        if (err & EEXIST) { ec = ec | ErrorCode::Eexist; }
        if (err & EINVAL) { ec = ec | ErrorCode::Einval; }
        if (err & ELOOP)  { ec = ec | ErrorCode::Eloop;  }
        if (err & ENOENT) { ec = ec | ErrorCode::EnoEnt; }
        if (err & ENOMEM) { ec = ec | ErrorCode::EnoMem; }
        if (err & ENOSPC) { ec = ec | ErrorCode::EnoSpc; }
        if (err & EPERM)  { ec = ec | ErrorCode::Eperm;  }
        if (err & EFAULT) { ec = ec | ErrorCode::Efault; }
        if (err & EINTR)  { ec = ec | ErrorCode::Eintr;  }
        if (err & EMFILE) { ec = ec | ErrorCode::EmFile; }
        if (err & ENFILE) { ec = ec | ErrorCode::EnFile; }
        if (err && ec == 0) { ec = ec | ErrorCode::Unknown; }

        return ec;
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
}