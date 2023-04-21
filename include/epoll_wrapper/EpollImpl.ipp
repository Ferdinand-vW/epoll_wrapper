#include "Epoll.h"
#include "EpollImpl.h"
#include "Event.h"
#include "Error.h"

namespace epoll_wrapper
{

    template <typename Epoll>
    CreateAction<Epoll>::CreateAction(std::unique_ptr<Epoll> epoll, ErrorCode errc)
    : mEpoll(std::move(epoll)), mErrc(errc) {};

    template <typename Epoll>
    bool CreateAction<Epoll>::hasError() const
    {
        return mErrc != ErrorCode::None;
    }

    template <typename Epoll>
    CreateAction<Epoll>::operator bool() const
    {
        return !hasError();
    }

    template <typename Epoll>
    ErrorCode CreateAction<Epoll>::getError() const
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

    CtlAction::CtlAction(ErrorCode errc) : mErrc(errc) {}
    
    bool CtlAction::hasError() const
    {
        return mErrc != ErrorCode::None;
    }

    ErrorCode CtlAction::getError() const
    {
        return mErrc;
    }

    template <typename FdType>
    WaitAction<FdType>::WaitAction(std::vector<std::pair<const FdType&, Event>>&& events, ErrorCode errc)
        : mEvents(std::move(events)), mErrc(errc) {}

    template <typename FdType>
    const std::vector<std::pair<const FdType&, Event>>& WaitAction<FdType>::getEvents() const
    {
        return mEvents;
    }

    template <typename FdType>
    bool WaitAction<FdType>::hasError() const
    {
        return mErrc != ErrorCode::None;
    }

    template <typename FdType>
    ErrorCode WaitAction<FdType>::getError() const
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
                , ErrorCode::None);
        }

        return CreateAction<Epoll>(nullptr, fromEpollError(errno));
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

        ErrorCode waErr{ErrorCode::None};

        if (resultCode < 0)
        {
            waErr = fromEpollError(errno);
        }

        while (i < resultCode)
        {
            Event ev{fromEpollEvent(events[i].events), fromEpollError(errno), std::move(events[i].data)};

            // Reserve event vector
            if (auto it = mRegisteredFds.find(events[i].data.fd); it != mRegisteredFds.end())
            {
                std::pair<const FdType&, Event> p = {it->second, std::move(ev)};
                eventVector.emplace_back(std::move(p));
            }
            
            ++i;
        }
        
        return WaitAction<FdType>{std::move(eventVector), waErr};
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

        auto ec = ErrorCode::None;
        if (res == 0)
        {
            mRegisteredEvents.insert({fd, eventc});
            mRegisteredFds.insert({fd, fdObj});

            auto ctl = CtlAction(ec);
        }
        else
        {
            ec = fromEpollError(errno);
        }

        return CtlAction{ec};
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
            return CtlAction{ErrorCode::EnoEnt};
        }

        struct epoll_event event;
        event.events = toEpollEvent(eventc);
        event.data.fd = fd;
        
        auto res = mEpoll->epoll_ctl(EPOLL_CTL_MOD, fd, &event);

        if (res == 0)
        {
            mRegisteredEvents[fd] = eventc;
            return CtlAction{ErrorCode::None};
        }

        return CtlAction{fromEpollError(errno)};
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
            return CtlAction{ErrorCode::None};
        }

        return CtlAction{fromEpollError(errno)};
    }
        
    template <typename EpollType, typename FdType>
    EpollType& EpollImpl<EpollType, FdType>::getUnderlying() const
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