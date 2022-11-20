#pragma once

#include <memory>
#include <optional>
#include <sys/epoll.h>

namespace epoll_wrapper
{
    class Light
    {
        private:
            int mEpollFd{0};

            Light(int epollfd);

        public:
            static std::unique_ptr<Light> epoll_create(int size);
            int epoll_ctl(int op, int fd, struct epoll_event *event);
            int epoll_wait(struct epoll_event *events, int maxevents, int timeout);
            void close();
            int getUnderlying() const;
    };
}