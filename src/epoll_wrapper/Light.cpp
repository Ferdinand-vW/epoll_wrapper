#include "epoll_wrapper/Light.h"

#include <memory>
#include <optional>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>

namespace epoll_wrapper
{
    Light::Light(int epollFd) : mEpollFd(epollFd) {}

    std::unique_ptr<Light> Light::epoll_create(int size)
    {
        int epollFd = ::epoll_create(size);

        return std::unique_ptr<Light>(new Light(epollFd));
    }

    int Light::epoll_ctl(int op, int fd, struct epoll_event *event)
    {
        return ::epoll_ctl(mEpollFd, op, fd, event);
    }

    int Light::epoll_wait(struct epoll_event *events, int maxevents, int timeout)
    {
        return ::epoll_wait(mEpollFd, events, maxevents, timeout);
    }

    void Light::close()
    {
        ::close(mEpollFd);
    }

    int Light::getUnderlying() const
    {
        return mEpollFd;
    }
}