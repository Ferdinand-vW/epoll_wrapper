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
    template <typename Epoll>
    class CreateAction
    {
        public:
            CreateAction(std::unique_ptr<Epoll> epoll, ErrorCode errc);

            operator bool() const;

            ErrorCode getError() const;

            bool hasError() const;

            Epoll& getEpoll();
            const Epoll& getEpoll() const;

        private:

        std::unique_ptr<Epoll> mEpoll; 
        ErrorCode mErrc;
    };

    class CtlAction
    {
        public:
            CtlAction(ErrorCode errc);

            bool hasError() const;

            ErrorCode getError() const;

        private:
            ErrorCode mErrc;
    };

    template <typename FdType>
    class WaitAction
    {
        public:
            WaitAction(std::vector<std::pair<const FdType&, Event>>&&, ErrorCode);

            const std::vector<std::pair<const FdType&, Event>>& getEvents() const;

            bool hasError() const;

            ErrorCode getError() const;

        private:
            std::vector<std::pair<const FdType&, Event>> mEvents;
            ErrorCode mErrc;
    };

    template <typename EpollType, typename FdType>
    class EpollImpl
    {
        
    public:
        static CreateAction<EpollImpl<EpollType, FdType>> epollCreate();

        EpollImpl(const EpollImpl&) = delete;
        EpollImpl& operator=(const EpollImpl&) = delete;
        EpollImpl& operator=(EpollImpl&&) = delete;

        WaitAction<FdType> wait(uint32_t timeout = -1);

        CtlAction add(const FdType& fd, EventCode event);
        CtlAction add(const FdType& fd, EventCodeMask event);
        CtlAction mod(const FdType& fd, EventCode event);
        CtlAction mod(const FdType& fd, EventCodeMask event);
        CtlAction erase(const FdType& fd);
        void close();

        const EpollType& getUnderlying() const;
        bool hasFd(uint32_t fd) const;
        const FdType& getFd(uint32_t fd) const;
        const EventCodeMask getEvents(const FdType &fd) const;
        
    private:
        static constexpr uint32_t MAXEVENTS = 32;

        int32_t mTimeout{-1};
        // uint32_t is the underlying file descriptor
        std::unordered_map<uint32_t, EventCodeMask> mRegisteredEvents;
        std::unordered_map<uint32_t, const FdType&> mRegisteredFds;
        struct epoll_event mEvents[MAXEVENTS];

        std::unique_ptr<EpollType> mEpoll;

        EpollImpl(std::unique_ptr<EpollType> epoll);
};
}