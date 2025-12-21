#pragma once
#include "Log.hpp"
#include "Socket.hpp"
#include "Connection.hpp"

#include <unordered_map>

#include <iostream>

class Reactor
{
public:
    Reactor(uint16_t port)
    {}

    void Init()
    {

    }

    void AddConnection(int sockfd)
    {
        // 1. 构建 connection
        Connection* con = new Connection(sockfd);

        // 2. 将 con 插入到connects
        _connects[sockfd] = con;

        // 3. 将新的 con 让epoll 管理
    }

    ~Reactor()
    {}
private:
    std::unordered_map<int, Connection*> _connects;
};