#pragma once

#include "EpollImpl.h"
#include "Light.h"
#include "Error.h"
#include "Event.h"

namespace epoll_wrapper
{
    template <typename FdType>
    using Epoll = EpollImpl<Light, FdType>;
}