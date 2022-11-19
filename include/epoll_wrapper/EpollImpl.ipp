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
#include <epoll_wrapper/Error.h>
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

    template <typename Epoll>
    CreateAction<Epoll>::CreateAction(std::unique_ptr<Epoll> epoll, ErrorCodeMask errc)
    : mEpoll(std::move(epoll)), mErrc(errc) {};

    template <typename Epoll>
    bool CreateAction<Epoll>::hasError() const
    {
        return mErrc;
    }

    template <typename Epoll>
    CreateAction<Epoll>::operator bool() const
    {
        return !hasError();
    }

    template <typename Epoll>
    ErrorCodeMask CreateAction<Epoll>::getError() const
    {
        return mErrc;
    }

    template <typename Epoll>
    Epoll& CreateAction<Epoll>::getEpoll()
    {
        return *mEpoll;
    }

    template <typename Epoll>
    const Epoll& CreateAction<Epoll>::getEpoll() const
    {
        return *mEpoll;
    }

    CtlAction::CtlAction(ErrorCodeMask errc) : mErrc(errc) {};
    
    bool CtlAction::hasError() const
    {
        return mErrc;
    }

    ErrorCodeMask CtlAction::getError() const
    {
        return mErrc;
    }

    template <typename FdType>
    WaitAction<FdType>::WaitAction(std::vector<std::pair<const FdType&, Event>>&& events, ErrorCodeMask errc)
        : mEvents(std::move(events)), mErrc(errc) {}

    template <typename FdType>
    const std::vector<std::pair<const FdType&, Event>>& WaitAction<FdType>::getEvents() const
    {
        return mEvents;
    }

    template <typename FdType>
    bool WaitAction<FdType>::hasError() const
    {
        return mErrc;
    }

    template <typename FdType>
    ErrorCodeMask WaitAction<FdType>::getError() const
    {
        return mErrc;
    }

    template <typename EpollType, typename FdType>
    EpollImpl<EpollType, FdType>::EpollImpl(std::unique_ptr<EpollType> epoll) : mEpoll(std::move(epoll)) {}

    template <typename EpollType, typename FdType>
    CreateAction<EpollImpl<EpollType, FdType>> EpollImpl<EpollType, FdType>::epollCreate()
    {
        using Epoll = EpollImpl<EpollType, FdType>;
        auto epollFd = EpollType::epoll_create(1);

        if (epollFd)
        {
            return CreateAction<Epoll>
                (std::unique_ptr<Epoll>(new EpollImpl(std::move(epollFd)))
                , ErrorCode::None | ErrorCode::None);
        }

        return CreateAction<Epoll>(nullptr, ErrorCode::None | ErrorCode::Unknown);
    }

    template <typename EpollType, typename FdType>
    void EpollImpl<EpollType, FdType>::close()
    {
        mEpoll.release();
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

            // Reserve event vector
            if (auto it = mRegisteredFds.find(events[i].data.fd); it != mRegisteredFds.end())
            {
                std::pair<const FdType&, Event> p = {it->second, std::move(ev)};
                eventVector.emplace_back(std::move(p));
            }
            
            ++i;
        }

        
        return WaitAction<FdType>{std::move(eventVector), ErrorCode::None | ErrorCode::None};
    }

    template <typename EpollType, typename FdType>
    CtlAction EpollImpl<EpollType, FdType>::add(const FdType& fd, EventCode event)
    {
        return add(fd, EventCode::None | event);
    }

    template <typename EpollType, typename FdType>
    CtlAction EpollImpl<EpollType, FdType>::add(const FdType& fdObj, EventCodeMask eventc)
    {
        auto fd = fdObj.getFileDescriptor();
        
        struct epoll_event event;
        event.events = toEpollEvent(eventc);
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
    CtlAction EpollImpl<EpollType, FdType>::mod(const FdType& fd, EventCode event)
    {
        return mod(fd, EventCode::None | event);
    }

    template <typename EpollType, typename FdType>
    CtlAction EpollImpl<EpollType, FdType>::mod(const FdType& fdObj, EventCodeMask eventc)
    {
        auto fd = fdObj.getFileDescriptor();

        if (mRegisteredFds.find(fd) == mRegisteredFds.end())
        {
            return CtlAction{ErrorCode::None | ErrorCode::EnoEnt};
        }

        struct epoll_event event;
        event.events = toEpollEvent(eventc);
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
    const EpollType& EpollImpl<EpollType, FdType>::getUnderlying() const
    {
        return *mEpoll;
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
    const EventCodeMask EpollImpl<EpollType, FdType>::getEvents(const FdType& fdObj) const
    {
        auto it  = mRegisteredEvents.find(fdObj.getFileDescriptor());

        if (it != mRegisteredEvents.end())
        {
            return it->second;
        }

        return 0;
    }
}