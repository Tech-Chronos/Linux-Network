#pragma once
#include "Log.hpp"
#include <iostream>
#include <functional>

class Connection;
using handler_t = std::function<void(Connection)>;

class Connection
{
public:
    Connection(int sockfd)
        :_sockfd(sockfd)
    {}

    

    ~Connection()
    {}
private:
    int _sockfd;

public:
    handler_t send_func;
    handler_t recv_func;
    handler_t except_func;
};