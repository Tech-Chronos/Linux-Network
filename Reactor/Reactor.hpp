#pragma once
#include "Socket.hpp"
#include "Connection.hpp"
#include "Multiplex.hpp"

#include <iostream>
#include <unordered_map>

const static int gsize = 128;

// reactor 就是要进行对事件的派发，有事件来的时候就会进行处理（单reactor，单线程）
class Reactor
{
public:
    Reactor()
        : _epoller(std::make_shared<Epoller>()), _isrunning(false)
    {
    }

    void AddConnection(int sockfd, uint32_t events, int type)
    {
        // 1. 创建对应的connection
        Connection *connect = new Connection(sockfd);

        // 1.1 将它关心的事件放入con
        connect->SetEvents(events);

        // 1.2 将它的类型放入con,监听 or 普通
        connect->SetType(type);

        // 1.3 将自身this放入con
        connect->SetReactor(this);

        // 2. 在Reactor的哈希表中进行管理
        _connects[sockfd] = connect;

        // 3. 注册con中的函数处理方法
        if (connect->GetType() == ListenConnection)
        {
            connect->_handler_recv = _listener;
            connect->_handler_send = nullptr;
            connect->_handler_except = nullptr;
            LOG(INFO, "create listener func success!");
        }
        else if (connect->GetType() == NormalConnection)
        {
            connect->_handler_recv = _recver;
            connect->_handler_send = _sender;
            connect->_handler_except = _excepter;
            LOG(INFO, "create normal func success!");
        }

        // 4. 将这个事件放入epoll内核管理
        _epoller->AddEvent(connect->GetSockFd(), connect->GetEvents());
    }

    void LoopOnce(int i)
    {
        // 获取就绪的事件的文件描述符和事件类型
        int sockfd = _event[i].data.fd;
        uint32_t ev = _event[i].events;

        // 将错误统一在except中处理
        if (ev & EPOLLHUP)
            ev |= (EPOLLIN | EPOLLET);
        if (ev & EPOLLERR)
            ev |= (EPOLLIN | EPOLLET);

        if (ev & EPOLLIN)
        {
            if (WhetherConnectExist(sockfd) && _connects[sockfd]->_handler_recv)
            {
                _connects[sockfd]->_handler_recv(_connects[sockfd]);
            }
        }
        else if (ev & EPOLLOUT)
        {
            if (WhetherConnectExist(sockfd) && _connects[sockfd]->_handler_send)
            {
                _connects[sockfd]->_handler_send(_connects[sockfd]);
            }
        }
    }

    void Dispatcher()
    {
        _isrunning = true;
        while (_isrunning)
        {
            int num = _epoller->EpollWait(_event, gsize);
            if (num > 0)
            {
                for (int i = 0; i < num; ++i)
                {
                    LoopOnce(i);
                    PrintDebug();
                }
            }
        }
        _isrunning = false;
    }

    void SetReadWriteEnable(int sockfd, bool readable, bool writeable)
    {
        if (WhetherConnectExist(sockfd))
        {
            int events = EPOLLET;
            events |= (((readable == true) ? EPOLLIN : 0) | ((writeable == true) ? EPOLLOUT : 0));

            if (_epoller->ModEvent(sockfd, events))
                LOG(INFO, "modify event %d successfully!", sockfd);
        }
    }

    bool WhetherConnectExist(int sockfd)
    {
        return _connects.find(sockfd) != _connects.end();
    }

    void PrintDebug()
    {
        std::string str;
        for (auto& it : _connects)
        {
            str += std::to_string(it.first);
            str += " ";
        }

        LOG(DEBUG, "管理的文件描述符队列：%s", str.c_str());
    }

    void SetFunc(handler_t recver, handler_t sender, handler_t excepter, handler_t listener)
    {
        _recver = recver;
        _sender = sender;
        _excepter = excepter;
        _listener = listener;
    }

    void DelConnection(int sockfd)
    {
        if (WhetherConnectExist(sockfd))
        {
            LOG(INFO, "socketfd %d has been erased!", sockfd);
            // 1. 从内核中移除对这个文件描述符的关心
            _epoller->DelEvent(sockfd);

            // 2. 关闭文件描述符
            close(sockfd);

            // 3. 将这个文件描述符从我的哈希表中移除
            _connects.erase(sockfd);
        }
    }

    ~Reactor()
    {
    }

private:
    std::unordered_map<int, Connection *> _connects;
    std::shared_ptr<Multiplex> _epoller;
    epoll_event _event[gsize];

    bool _isrunning;

public:
    handler_t _recver;
    handler_t _sender;
    handler_t _excepter;
    handler_t _listener;
};