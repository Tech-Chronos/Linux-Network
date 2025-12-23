#pragma once
#include <sys/epoll.h>

const static int gnum = 128;
class Multiplex
{
public:
    virtual bool AddEvent(int sockfd, int events) = 0;
    virtual bool ModEvent(int sockfd, int events) = 0;
    virtual int EpollWait(epoll_event* event, int maxnum, int timeout = -1) = 0;

    virtual bool DelEvent(int sockfd) = 0;
    
};

class Epoller : public Multiplex
{
private:
    epoll_event PollHelper(int sockfd, int events)
    {
        epoll_event event;
        event.data.fd = sockfd;
        event.events = events;

        return event;
    }
public:
    Epoller()
    {
        _epfd = epoll_create(gnum);
    }

    bool AddEvent(int sockfd, int events) override
    {
        epoll_event event = PollHelper(sockfd, events);
        
        return (epoll_ctl(_epfd, EPOLL_CTL_ADD, sockfd, &event) == 0) ? true : false;
    }

    bool ModEvent(int sockfd, int events) override
    {
        epoll_event event = PollHelper(sockfd, events);

        return (epoll_ctl(_epfd, EPOLL_CTL_MOD, sockfd, &event) == 0) ? true : false;
    }

    bool DelEvent(int sockfd) override
    {
        return (epoll_ctl(_epfd, EPOLL_CTL_DEL, sockfd, nullptr) == 0) ? true : false;
    }

    int EpollWait(epoll_event event[], int maxnum, int timeout = -1) override
    {
        int num = epoll_wait(_epfd, event, maxnum, timeout);
        return num;
    }

private:
    int _epfd;
};