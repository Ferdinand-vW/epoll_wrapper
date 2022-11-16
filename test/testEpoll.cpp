#include "epoll_wrapper/Epoll.h"
#include "epoll_wrapper/Light.h"
#include "epoll_wrapper/EpollImpl.ipp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <optional>
#include <sstream>
#include <sys/epoll.h>
#include <unistd.h>

using namespace epoll_wrapper;

struct MockEpoll
{
    MockEpoll()
    {

    }

    MOCK_METHOD(int,epoll_ctl,(int, int, struct epoll_event*));
    MOCK_METHOD(int,epoll_wait,(struct epoll_event *events, int, int));
    MOCK_METHOD(void,close,());

    static std::unique_ptr<MockEpoll> epoll_create(int size)
    {
        return std::make_unique<MockEpoll>();
    }

};

struct Fd
{
    int32_t fd;

    public:
    int32_t getFileDescriptor() const
    {
        return fd;
    }
};

int READSIZE=64*1024;

void write_to_pipe (int fd, std::string text)
{
  FILE *stream;
  stream = fdopen (fd, "w");
  fprintf (stream, "%s", text.c_str());
  fclose (stream);
}

TEST(EPOLL, create_delete)
{
    auto createEpoll = EpollImpl<Light, int>::epollCreate();

    ASSERT_EQ(createEpoll.getError(), ErrorCode::None);
    ASSERT_TRUE(createEpoll);

    auto &epoll = createEpoll.getEpoll();

    EXPECT_NO_THROW(epoll.close());
}

TEST(EPOLL, add_and_remove_listener)
{
    auto createEpoll = EpollImpl<Light, Fd>::epollCreate();

    ASSERT_EQ(createEpoll.getError(), ErrorCode::None);
    ASSERT_TRUE(createEpoll);

    auto &epoll = createEpoll.getEpoll();

    auto std_in = Fd{0};
    auto res = epoll.add(std_in, EventCode::EpollIn);

    std::cout << ErrorCode::None << std::endl;

    ASSERT_EQ(res.getError(), ErrorCode::None);

    auto res2 = epoll.erase(std_in);

    ASSERT_EQ(res2.getError(), ErrorCode::None);

    auto fd = Fd{1};
    auto res3 = epoll.erase(fd);

    ASSERT_EQ(res3.getError(), ErrorCode::Einval);

    EXPECT_NO_THROW((void)epoll.close());
}

TEST(EPOLL, wait)
{
    auto createEpoll = EpollImpl<Light, Fd>::epollCreate();

    ASSERT_EQ(createEpoll.getError(), ErrorCode::None);
    ASSERT_TRUE(createEpoll);

    auto &epoll = createEpoll.getEpoll();

    int mypipe[2];
    int pipeRes = pipe(mypipe);

    ASSERT_EQ(pipeRes, 0);

    auto readFd = Fd{mypipe[0]};
    auto res = epoll.add(readFd, EventCode::EpollIn);

    ASSERT_EQ(res.getError(), ErrorCode::None);
    
    std::string input("test");
    write_to_pipe(mypipe[1], input);

    auto waitResult = epoll.wait();
    const auto events = waitResult.getEvents();

    ASSERT_EQ(waitResult.getError(), ErrorCode::None);
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.front().second.mData.fd, readFd.getFileDescriptor());
    
    char read_buf[READSIZE];
    int bytes_read = read(events.front().second.mData.fd, read_buf, READSIZE);

    ASSERT_EQ(bytes_read, input.size());

    std::string s(read_buf);

    ASSERT_EQ(s, input);
}

TEST(EPOLL, wait_empty_input)
{
    auto createEpoll = EpollImpl<Light, Fd>::epollCreate();

    ASSERT_EQ(createEpoll.getError(), ErrorCode::None);
    ASSERT_TRUE(createEpoll);

    auto &epoll = createEpoll.getEpoll();

    int mypipe[2];
    int pipeRes = pipe(mypipe);

    ASSERT_EQ(pipeRes, 0);

    auto readFd = Fd{mypipe[0]};
    auto res = epoll.add(readFd, EventCode::EpollIn);

    ASSERT_EQ(res.getError(), ErrorCode::None);

    auto waitResult = epoll.wait(0);
    const auto events = waitResult.getEvents();

    ASSERT_EQ(waitResult.getError(), ErrorCode::None);
    ASSERT_EQ(events.size(), 0);    
}

TEST(EPOLL, mod_fd_does_not_exist)
{
    auto createEpoll = EpollImpl<Light, Fd>::epollCreate();

    ASSERT_EQ(createEpoll.getError(), ErrorCode::None);
    ASSERT_TRUE(createEpoll);

    auto &epoll = createEpoll.getEpoll();

    auto readFd = Fd{0};

    auto ctl_res = epoll.mod(readFd, EventCode::EpollOut);

    ASSERT_EQ(ctl_res.getError(), ErrorCode::EnoEnt);
}

TEST(EPOLL, consistent_state)
{
    auto createEpoll = EpollImpl<Light, Fd>::epollCreate();

    ASSERT_EQ(createEpoll.getError(), ErrorCode::None);
    ASSERT_TRUE(createEpoll);

    auto &epoll = createEpoll.getEpoll();

    auto fd1 = Fd{0};
    auto fd2 = Fd{1};

    ASSERT_FALSE(epoll.hasFd(fd1.getFileDescriptor()));
    ASSERT_FALSE(epoll.hasFd(fd2.getFileDescriptor()));
    ASSERT_TRUE(epoll.getEvents(fd1).mCodes.empty());
    ASSERT_TRUE(epoll.getEvents(fd2).mCodes.empty());

    epoll.add(fd1, EventCode::EpollErr);
    ASSERT_TRUE(epoll.hasFd(fd1.getFileDescriptor()));
    ASSERT_FALSE(epoll.hasFd(fd2.getFileDescriptor()));
    ASSERT_FALSE(epoll.getEvents(fd1).mCodes.empty());
    ASSERT_TRUE(epoll.getEvents(fd2).mCodes.empty());

    epoll.add(fd2, EventCode::EpollErr);
    ASSERT_TRUE(epoll.hasFd(fd1.getFileDescriptor()));
    ASSERT_TRUE(epoll.hasFd(fd2.getFileDescriptor()));
    ASSERT_FALSE(epoll.getEvents(fd1).mCodes.empty());
    ASSERT_FALSE(epoll.getEvents(fd2).mCodes.empty());

    epoll.erase(fd1);
    ASSERT_FALSE(epoll.hasFd(fd1.getFileDescriptor()));
    ASSERT_TRUE(epoll.hasFd(fd2.getFileDescriptor()));
    ASSERT_TRUE(epoll.getEvents(fd1).mCodes.empty());
    ASSERT_FALSE(epoll.getEvents(fd2).mCodes.empty());

    epoll.erase(fd2);
    ASSERT_FALSE(epoll.hasFd(fd1.getFileDescriptor()));
    ASSERT_FALSE(epoll.hasFd(fd2.getFileDescriptor()));
    ASSERT_TRUE(epoll.getEvents(fd1).mCodes.empty());
    ASSERT_TRUE(epoll.getEvents(fd2).mCodes.empty());

}

TEST(EPOLL, mod_update_events)
{
    auto createEpoll = EpollImpl<MockEpoll, Fd>::epollCreate();

    ASSERT_EQ(createEpoll.getError(), ErrorCode::None);
    ASSERT_TRUE(createEpoll);

    auto &epoll = createEpoll.getEpoll();

    auto readFd = Fd{0};

    const auto &underling = epoll.getUnderlying();
    EXPECT_CALL(underling,epoll_ctl).Times(4);

    ASSERT_EQ(epoll.getEvents(readFd), EventCodes({}));

    auto res1 = epoll.add(readFd, EventCode::EpollIn);
    ASSERT_EQ(res1.getError(), ErrorCode::None);

    ASSERT_EQ(epoll.getEvents(readFd), EventCode::EpollIn);

    auto res2 = epoll.mod(readFd, EventCode::EpollErr);
    ASSERT_EQ(res2.getError(), ErrorCode::None);

    ASSERT_EQ(epoll.getEvents(readFd), EventCode::EpollErr);

    auto res3 = epoll.mod(readFd, EventCode::EpollErr | EventCode::EpollIn);
    ASSERT_EQ(res3.getError(), ErrorCode::None);

    ASSERT_EQ(epoll.getEvents(readFd), EventCode::EpollErr | EventCode::EpollIn);

    auto res4 = epoll.erase(readFd);
    ASSERT_EQ(res4.getError(), ErrorCode::None);

    ASSERT_EQ(epoll.getEvents(readFd), EventCodes({}));

}

