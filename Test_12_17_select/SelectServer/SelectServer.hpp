#include "Log.hpp"
#include "Socket.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <memory>

#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

class SelectServer
{
private:
    void HandlerEvent(fd_set& table)
    {
        if (FD_ISSET(_listen_sock->GetSockFd(), &table))
        {
            InAddr client;
            SockPtr conn_fd = _listen_sock->ServerAccept(&client);

            LOG(INFO, "accept client, conn_fd = %d, addr = %s:%d", conn_fd->GetSockFd(), client.GetIP().c_str(), client.GetPort());
        }
    }
public:
    SelectServer(uint16_t port)
        :_port(port)
        ,_listen_sock(std::make_unique<TcpSocket>())
    {
        _listen_sock->TcpServerCreateSocket(port);
    }

    void Init()
    {}

    void Loop()
    {
        while (true)
        {
            fd_set tables;
            FD_ZERO(&tables);
            FD_SET(_listen_sock->GetSockFd(), &tables);

            timeval cir_time = {5, 0};
            int ret = select(_listen_sock->GetSockFd() + 1, &tables, nullptr, nullptr, &cir_time);
            if (ret > 0)
            {
                LOG(INFO, "有事件就绪！cir_time = %d, %d, ret = %d", cir_time.tv_sec,cir_time.tv_usec, ret);
                HandlerEvent(tables);
                continue;
            }
            else if (ret == 0)
            {
                LOG(INFO, "无事件就绪！cir_time = %d, %d, ret = %d", cir_time.tv_sec,cir_time.tv_usec, ret);
                continue;
            }
            else
            {
                LOG(FATAL, "fd error!");
                break;
            }
        }
    }

    ~SelectServer()
    {}
private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listen_sock;
};


