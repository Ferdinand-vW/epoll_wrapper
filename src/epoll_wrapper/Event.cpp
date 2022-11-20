#include "epoll_wrapper/Event.h"
#include <iostream>
#include <sys/epoll.h>

namespace epoll_wrapper
{
    std::ostream& operator<<(std::ostream& os, const EventCode& ec)
    {
        switch(ec)
        {
        case EventCode::EpollIn:
            os << "EPOLLIN";
            break;
        case EventCode::EpollOut:
            os << "EPOLLOUT";
            break;
        case EventCode::EpollRdHUp:
            os << "EPOLLRDHUP";
            break;
        case EventCode::EpollPri:
            os << "EPOLLPRI";
            break;
        case EventCode::EpollErr:
            os << "EPOLLERR";
            break;
        case EventCode::EpollHUp:
            os << "EPOLLHUP";
            break;
        case EventCode::EpollEt:
            os << "EPOLLET";
            break;
        case EventCode::EpollOneShot:
            os << "EPOLLONESHOT";
            break;
        case EventCode::EpollWakeUp:
            os << "EPOLLWAKEUP";
            break;
        case EventCode::EpollExclusive:
            os << "EPOLLEXCLUSIVE";
            break;
        case EventCode::None:
            os << "NONE";
            break;
        }

        return os;
    }

    constexpr int fromEvent(EventCode event)
    {
        switch (event)
        {
            case EventCode::EpollIn:
                return EPOLLIN;
            case EventCode::EpollOut:
                return EPOLLOUT;
            case EventCode::EpollRdHUp:
                return EPOLLRDHUP;
            case EventCode::EpollPri:
                return EPOLLPRI;
            case EventCode::EpollErr:
                return EPOLLERR;
            case EventCode::EpollHUp:
                return EPOLLHUP;
            case EventCode::EpollEt:
                return EPOLLET;
            case EventCode::EpollOneShot:
                return EPOLLONESHOT;
            case EventCode::EpollWakeUp:
                return EPOLLWAKEUP;
            case EventCode::EpollExclusive:
                return EPOLLEXCLUSIVE;
            case EventCode::None:
                return 0;
        }

        return 0;
    }

    int toEpollEvent(EventCodeMask ec)
    {
        int eventc = 0;

        if (ec & EventCode::EpollIn)        { eventc = eventc | EPOLLIN;        }
        if (ec & EventCode::EpollOut)       { eventc = eventc | EPOLLOUT;       }
        if (ec & EventCode::EpollRdHUp)     { eventc = eventc | EPOLLRDHUP;     }
        if (ec & EventCode::EpollPri)       { eventc = eventc | EPOLLPRI;       }
        if (ec & EventCode::EpollErr)       { eventc = eventc | EPOLLERR;       }
        if (ec & EventCode::EpollHUp)       { eventc = eventc | EPOLLHUP;       }
        if (ec & EventCode::EpollEt)        { eventc = eventc | EPOLLET;        }
        if (ec & EventCode::EpollOneShot)   { eventc = eventc | EPOLLONESHOT;   }
        if (ec & EventCode::EpollWakeUp)    { eventc = eventc | EPOLLWAKEUP;    }
        if (ec & EventCode::EpollExclusive) { eventc = eventc | EPOLLEXCLUSIVE; }

        return eventc;
    }

    EventCodeMask fromEpollEvent(int eventc)
    {
        EventCodeMask ec;

        if (eventc & EPOLLIN)        { ec = ec | EventCode::EpollIn;        }
        if (eventc & EPOLLOUT)       { ec = ec | EventCode::EpollOut;       }
        if (eventc & EPOLLRDHUP)     { ec = ec | EventCode::EpollRdHUp;     }
        if (eventc & EPOLLPRI)       { ec = ec | EventCode::EpollPri;       }
        if (eventc & EPOLLERR)       { ec = ec | EventCode::EpollErr;       }
        if (eventc & EPOLLHUP)       { ec = ec | EventCode::EpollHUp;       }
        if (eventc & EPOLLET)        { ec = ec | EventCode::EpollEt;        }
        if (eventc & EPOLLONESHOT)   { ec = ec | EventCode::EpollOneShot;   }
        if (eventc & EPOLLWAKEUP)    { ec = ec | EventCode::EpollWakeUp;    }
        if (eventc & EPOLLEXCLUSIVE) { ec = ec | EventCode::EpollExclusive; }

        return ec;
    }

    EventCodeMask operator|(EventCodeMask ec1, EventCode ec2)
    {
        return ec1 | static_cast<EventCodeMask>(ec2);
    }

    EventCodeMask operator|(EventCode ec1, EventCode ec2)
    {
        return static_cast<EventCodeMask>(ec1) | ec2;
    }

    EventCodeMask operator&(EventCodeMask ec1, EventCode ec2)
    {
        return ec1 & static_cast<EventCodeMask>(ec2);
    }

    EventCodeMask operator&(EventCode ec1, EventCode ec2)
    {
        return static_cast<EventCodeMask>(ec1) & ec2;
    }
}