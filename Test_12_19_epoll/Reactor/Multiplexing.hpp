#include "Log.hpp"

#include <sys/epoll.h>

static const int gnum = 128;
class Multiplex
{
public:
    virtual bool AddEvent(int sockfd, int ev) = 0;
    virtual int Wait(epoll_event revent[], int maxevent, int timeout = -1) = 0;
    virtual bool ModifyEvent(int sockfd, int ev) = 0;
    virtual bool DelEvent(int sockfd) = 0;
};

class Epoller : public Multiplex
{
private:    
    epoll_event EventHelper(int sockfd, int ev)
    {
        epoll_event event;
        event.data.fd = sockfd;
        event.events = ev;
        
        return event;
    }
public:
    Epoller()
    {
        _epfd = epoll_create(gnum);
    }

    bool AddEvent(int sockfd, int ev) override
    {
        epoll_event event = EventHelper(sockfd, ev);
        int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, sockfd, &event);
        return ret == 0 ? true : false;
    }

    bool ModifyEvent(int sockfd, int ev) override
    {
        epoll_event event = EventHelper(sockfd, ev);
        int ret = epoll_ctl(_epfd, EPOLL_CTL_MOD, sockfd, &event);
        return ret == 0 ? true : false;
    }

    bool DelEvent(int sockfd) override
    {
        int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, sockfd, nullptr);
        return ret == 0 ? true : false;
    }

    int Wait(epoll_event revent[], int maxevent, int timeout)
    {
        int ret = epoll_wait(_epfd, revent, maxevent, timeout);
        if (ret > 0)
        {
            LOG(INFO, "epoll_wait success, prepared num is %d", ret);
        }
        else if (ret == 0)
        {
            LOG(INFO, "no event prepared!");
        }
        else
        {
            LOG(ERROR, "epoll wait error!");
        }
        return ret;
    }

private:
    int _epfd;
};