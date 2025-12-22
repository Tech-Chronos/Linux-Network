#pragma once
#include "Connection.hpp"
#include "PackgeParse.hpp"

const static int bufsize = 1024;

class HandlerIO
{
public:
    HandlerIO(handler_t handler_package)
        :_handle_package(handler_package)
    {}

    void RecvMessage(Connection* con)
    {
        LOG(INFO,"recv message from %s:%d!",con->GetAddr().GetIP().c_str(), con->GetAddr().GetPort());

        while (true)
        {
            char buffer[bufsize];
            int ret = recv(con->GetSockFD(), buffer, bufsize, 0);
            if (ret > 0)
            {
                buffer[ret] = 0;
                con->Inbuffer() += buffer;
            }
            else if (ret == 0)
            {
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
                else if (errno = EINTR)
                {
                    LOG(INFO, "be interupted by signal!");
                    continue;
                }
                else
                {
                    LOG(ERROR, "read error!");
                    return;
                }
            }
        }
        _handle_package(con);
    }
    void SendMessage(Connection* con)
    {

    }
    void HandleExcept(Connection* con)
    {

    }
private:
    handler_t _handle_package;
};