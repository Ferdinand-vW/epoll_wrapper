#pragma once

#include "Error.h"
#include "Event.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <sys/epoll.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace epoll_wrapper
{
    template<typename EpollType, typename FdType>
    class EpollImpl;

    template <typename EpollType, typename FdType>
    struct CreateAction
    {
        std::unique_ptr<EpollImpl<EpollType, FdType>> mEpoll; 
        ErrorCode mErrc;

        EpollImpl<EpollType, FdType>* operator->()
        {
            return mEpoll.get();
        }

        bool hasError();
    };

    struct CtlAction
    {
        ErrorCode mErrc;

        bool hasError();
    };

    template <typename FdType>
    struct WaitAction
    {
        std::vector<std::pair<const FdType&, Event>> mEvents;
        ErrorCode mErrc;

        std::vector<std::pair<const FdType&, Event>>* operator->()
        {
            return &mEvents;
        }

        bool haError();
    };

    template <typename EpollType, typename FdType>
    class EpollImpl
    {
        
    public:
        static CreateAction<EpollType, FdType> epollCreate();

        ~EpollImpl();

        EpollImpl(const EpollImpl&) = delete;
        EpollImpl(EpollImpl&&) = delete;
        EpollImpl& operator=(const EpollImpl&) = delete;
        EpollImpl& operator=(EpollImpl&&) = delete;

        WaitAction<FdType> wait(uint32_t timeout = -1);

        CtlAction add(const FdType& fd, EventCodes event);
        CtlAction mod(const FdType& fd, EventCodes event);
        CtlAction erase(const FdType& fd);
        void close();

        const std::unique_ptr<EpollType>& getUnderlying() const;
        bool hasFd(uint32_t fd) const;
        const FdType& getFd(uint32_t fd) const;
        const EventCodes getEvents(const FdType &fd) const;
        
    private:
        static constexpr uint32_t MAXEVENTS = 32;

        int32_t mTimeout{-1};
        // uint32_t is the underlying file descriptor
        std::unordered_map<uint32_t, EventCodes> mRegisteredEvents;
        std::unordered_map<uint32_t, const FdType&> mRegisteredFds;
        struct epoll_event mEvents[MAXEVENTS];

        std::unique_ptr<EpollType> mEpoll;

        EpollImpl(std::unique_ptr<EpollType>&& epollfd);
};
}