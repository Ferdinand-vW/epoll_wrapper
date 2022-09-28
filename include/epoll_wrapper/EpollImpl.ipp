#include "Epoll.h"
#include "Event.h"
#include "Error.h"

#include <algorithm>
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <epoll_wrapper/EpollImpl.h>
#include <exception>
#include <iostream>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_set>

namespace epoll_wrapper
{

    template <typename EpollType, typename FdType>
    bool CreateAction<EpollType, FdType>::hasError()
    {
        return mErrc != ErrorCode::None;
    }

    bool CtlAction::hasError()
    {
        return mErrc != ErrorCode::None;
    }

    template <typename FdType>
    bool WaitAction<FdType>::haError()
    {
        return mErrc != ErrorCode::None;
    }

    template <typename EpollType, typename FdType>
    EpollImpl<EpollType, FdType>::EpollImpl(std::unique_ptr<EpollType>&& epoll) : mEpoll(std::move(epoll)) {}

    template <typename EpollType, typename FdType>
    CreateAction<EpollType, FdType> EpollImpl<EpollType, FdType>::epollCreate()
    {
        auto epollFd = EpollType::epoll_create(1);

        if (epollFd)
        {
            return CreateAction<EpollType, FdType>{
                std::unique_ptr<EpollImpl<EpollType, FdType>>(
                    new EpollImpl<EpollType, FdType>(std::move(epollFd)))
                , ErrorCode::None};
        }

        return {};
    }

    template <typename EpollType, typename FdType>
    EpollImpl<EpollType, FdType>::~EpollImpl()
    {
        this->close();
    }

    template <typename EpollType, typename FdType>
    void EpollImpl<EpollType, FdType>::close()
    {
        mEpoll->close();
    }

    template <typename EpollType, typename FdType>
    WaitAction<FdType> EpollImpl<EpollType, FdType>::wait(uint32_t timeout)
    {
        struct epoll_event events[MAXEVENTS];
        auto resultCode = mEpoll->epoll_wait(events, MAXEVENTS, timeout);

        std::vector<std::pair<const FdType&, Event>> eventVector;
        int i = 0;

        while (i < resultCode)
        {
            Event ev{fromEpollEvent(events[i].events), fromEpollError(resultCode), std::move(events[i].data)};
            
            if (auto it = mRegisteredFds.find(events[i].data.fd); it != mRegisteredFds.end())
            {
                std::pair<const FdType&, Event> p = {it->second, std::move(ev)};
                eventVector.emplace_back(p);
            }
            
            ++i;
        }

        
        return WaitAction<FdType>{eventVector, ErrorCode::None};
    }

    template <typename EpollType, typename FdType>
    CtlAction EpollImpl<EpollType, FdType>::add(const FdType& fdObj, EventCodes eventc)
    {
        auto fd = fdObj.getFileDescriptor();
        
        struct epoll_event event;
        event.events = fromEvent(eventc);
        event.data.fd = fd;
        
        auto res = mEpoll->epoll_ctl(EPOLL_CTL_ADD, fd, &event);

        if (res == 0)
        {
            mRegisteredEvents.insert({fd, eventc});
            mRegisteredFds.insert({fd, fdObj});
        }

        return CtlAction{fromEpollError(res)};
    }

    template <typename EpollType, typename FdType>
    CtlAction EpollImpl<EpollType, FdType>::mod(const FdType& fdObj, EventCodes eventc)
    {
        auto fd = fdObj.getFileDescriptor();

        if (mRegisteredFds.find(fd) == mRegisteredFds.end())
        {
            return CtlAction{ErrorCode::EnoEnt};
        }

        struct epoll_event event;
        event.events = fromEvent(eventc);
        event.data.fd = fd;
        
        auto res = mEpoll->epoll_ctl(EPOLL_CTL_MOD, fd, &event);

        if (res == 0)
        {
            mRegisteredEvents[fd] = eventc;
        }

        return CtlAction{fromEpollError(res)};
    }

    template <typename EpollType, typename FdType>
    CtlAction EpollImpl<EpollType, FdType>::erase(const FdType& fdObj)
    {
        auto fd = fdObj.getFileDescriptor();

        struct epoll_event event;
        auto res = mEpoll->epoll_ctl(EPOLL_CTL_DEL, fd, &event);

        if (res == 0)
        {
            mRegisteredEvents.erase(fd);
            mRegisteredFds.erase(fd);
        }

        return CtlAction{fromEpollError(res)};
    }
        
    template <typename EpollType, typename FdType>
    const std::unique_ptr<EpollType>& EpollImpl<EpollType, FdType>::getUnderlying() const
    {
        return mEpoll;
    }

    template <typename EpollType, typename FdType>
    bool EpollImpl<EpollType, FdType>::hasFd(uint32_t fd) const
    {
        return mRegisteredFds.find(fd) != mRegisteredFds.end();
    }

    template <typename EpollType, typename FdType>
    const FdType& EpollImpl<EpollType, FdType>::getFd(uint32_t fd) const
    {
        auto it = mRegisteredFds.find(fd);

        if (it != mRegisteredFds.end())
        {
            return it->second;
        }

        return {};
    }
    
    template <typename EpollType, typename FdType>
    const EventCodes EpollImpl<EpollType, FdType>::getEvents(const FdType& fdObj) const
    {
        auto it  = mRegisteredEvents.find(fdObj.getFileDescriptor());

        if (it != mRegisteredEvents.end())
        {
            return it->second;
        }

        return EventCodes(std::unordered_set<EventCode>{});
    }
}