#pragma once
#include "Connection.hpp"
#include "PackgeParse.hpp"

const static int bufsize = 1024;

class Reactor;
class HandlerIO
{
public:
    HandlerIO(handler_t handler_package)
        : _handle_package(handler_package)
    {
    }

    void RecvMessage(Connection *con)
    {
        LOG(INFO, "recv message from %s:%d!", con->GetAddr().GetIP().c_str(), con->GetAddr().GetPort());

        while (true)
        {
            char buffer[bufsize];
            int ret = recv(con->GetSockFD(), buffer, bufsize - 1, 0);
            if (ret > 0)
            {
                buffer[ret] = 0;
                con->Inbuffer() += buffer;
            }
            else if (ret == 0)
            {
                con->except_func(con);

                LOG(INFO, "client quit!");
                break;
            }
            else
            {
                if (errno == EWOULDBLOCK)
                {
                    LOG(INFO, "data has been read done!");
                    break;
                }
                else if (errno == EINTR)
                {
                    LOG(INFO, "be interupted by signal!");
                    continue;
                }
                else
                {
                    con->except_func(con);
                    LOG(ERROR, "read error!");
                    return;
                }
            }
        }
        _handle_package(con);
    }

    void SendMessage(Connection *con)
    {
        errno = 0;
        while (true)
        {
            int n = send(con->GetSockFD(), con->Outbuffer().c_str(), con->Outbuffer().size(), 0);
            if (n > 0)
            {
                con->Outbuffer().erase(0, n);
                if (con->Outbuffer().empty())
                    break;
            }
            else if (n == 0)
            {
                break;
            }
            else
            {
                if (errno == EWOULDBLOCK)
                {
                    LOG(INFO, "send block!");
                    // 将 这个connect 的 EPOLLOUT 设置进入内核
                    con->_R->SetWriteReadEnable(con, true, true);
                    break;
                }
                else if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    LOG(ERROR, "send error!");
                    con->except_func(con);
                    return;
                }
            }
        }
    }
    void HandleExcept(Connection *con)
    {
        con->_R->DeleSock(con);
    }

private:
    handler_t _handle_package;
};