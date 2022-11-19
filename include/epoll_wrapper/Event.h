#pragma once

#include "Error.h"

#include <ostream>
#include <sys/types.h>
#include <vector>
#include <sys/epoll.h>
#include <unordered_set>

namespace epoll_wrapper
{
    using EventCodeMask = u_int16_t;
    enum class EventCode : EventCodeMask
        { None           = 0u
        , EpollIn        = 1u << 0
        , EpollOut       = 1u << 1
        , EpollRdHUp     = 1u << 2
        , EpollPri       = 1u << 3
        , EpollErr       = 1u << 4
        , EpollHUp       = 1u << 5
        , EpollEt        = 1u << 6
        , EpollOneShot   = 1u << 7
        , EpollWakeUp    = 1u << 8
        , EpollExclusive = 1u << 9
        };

    std::ostream& operator<<(std::ostream&, const EventCode&);
 
    EventCodeMask operator|(EventCodeMask ec1, EventCode ec2);
    EventCodeMask operator|(EventCode ec1, EventCode ec2);
    EventCodeMask operator&(EventCodeMask ec1, EventCode ec2);
    EventCodeMask operator&(EventCode ec1, EventCode ec2);

    struct Event
    {
        EventCodeMask mCodes;
        ErrorCodeMask mError;
        epoll_data_t mData;
        uint32_t mFd;
    };

    // int fromEvent(const EventCode& events);
    int toEpollEvent(EventCodeMask event);
    EventCodeMask fromEpollEvent(int eventc);
}