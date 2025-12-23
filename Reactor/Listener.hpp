#pragma once
#include "Socket.hpp"
#include "Connection.hpp"

class Listener
{
public:
    Listener(uint16_t port)
        : _port(port)
        , _listen_sock(std::make_shared<TcpSocket>())
    {
        _listen_sock->TcpServerCreateSocket(_port);
    }

    int GetListenSockfd()
    {
        return _listen_sock->GetSockFd();
    }

    void Acceptor(Connection *con)
    {
        while (true)
        {
            InAddr addr;
            int code = 0;
            SockPtr connect_fd = _listen_sock->ServerAccept(&addr, &code);

            if (connect_fd != nullptr)
            {
                LOG(INFO, "new link arrival! addr is %s:%d", addr.GetIP().c_str(), addr.GetPort());
                con->GetReactor()->AddConnection(connect_fd->GetSockFd(), EPOLLIN | EPOLLET, NormalConnection);
            }
            else
            {
                if (code == EWOULDBLOCK)
                {
                    LOG(INFO, "link has been read completely!");
                    break;
                }
                else if (code == EINTR)
                {
                    LOG(INFO, "be interrupted by the signal!");
                    continue;
                }
                else
                {
                    LOG(ERROR, "accept error!");
                    break;
                }
            }
        }
    }

private:
    uint16_t _port;
    SockPtr _listen_sock;
};