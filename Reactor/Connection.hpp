#pragma once

#include <sys/epoll.h>
#include <functional>
#include <string>

#define ListenConnection 0
#define NormalConnection 1

class Reactor;
class Connection;
using handler_t = std::function<void(Connection*)>;

// 链接中应该包含相应的方法、文件描述符以及对应的事件
class Connection
{
public:
    Connection(int sockfd)
        :_sockfd(sockfd)
    {}

    int GetSockFd()
    {
        return _sockfd;
    }

    uint32_t GetEvents()
    {
        return _events;
    }

    int GetType()
    {
        return _type;
    }

    Reactor* GetReactor()
    {
        return _R;
    }

    std::string& GetInbuffer()
    {
        return _inbuffer;
    }

    std::string& GetOutbuffer()
    {
        return _outbuffer;
    }

    void SetEvents(uint32_t events)
    {
        _events = events;
    }

    void SetReactor(Reactor* R)
    {
        _R = R;
    }

    void SetType(int type)
    {
        _type = type;
    }

    // 将方法主次进入 connection
    void RegisterHandlerFunc(handler_t recver, handler_t sender, handler_t excepter)
    {
        _handler_recv = recver;
        _handler_send = sender;
        _handler_except = excepter;
    }


private:
    int _sockfd;
    uint32_t _events;
    Reactor* _R;
    int _type;

    std::string _inbuffer;
    std::string _outbuffer;

public:
    handler_t _handler_recv;
    handler_t _handler_send;
    handler_t _handler_except;
};