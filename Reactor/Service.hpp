#pragma once
#include "Connection.hpp"
#include "Log.hpp"

#include <unistd.h>
#include <sys/socket.h>

const static int maxsize = 1024;
class HandlerIO
{
public:
    HandlerIO(handler_t process_parse)
        :_process_parse(process_parse)
    {}

    void RecvMessage(Connection* con)
    {
        while (true)
        {
            errno = 0;
            char buffer[maxsize];
            int ret = recv(con->GetSockFd(), buffer, maxsize - 1, 0);
            if (ret > 0)
            {
                buffer[ret] = 0;
                LOG(INFO, "content is %s", buffer);
                con->GetInbuffer() += buffer;
            }
            else if (ret == 0)
            {
                LOG(INFO, "client quit!");
                HandleExcept(con);
            }
            else
            {
                if (errno == EWOULDBLOCK)
                {
                    LOG(INFO, "message has been read completely!");
                    break;
                }
                else if (errno == EINTR)
                {
                    LOG(INFO, "be interrupted by the signal!");
                    continue;
                }
                else 
                {
                    LOG(ERROR, "recv error!");
                    HandleExcept(con);
                    return;
                }
            }
        }
        // 如果不为空，就判断报文是否是完整的，然后进行业务处理
        if (!con->GetInbuffer().empty())
        {
            _process_parse(con);
        }
    }

    void SendMessage(Connection* con)
    {
        while (true)
        {
            errno = 0;
            int ret = send(con->GetSockFd(), con->GetOutbuffer().c_str(), con->GetOutbuffer().size(), 0);
            if (ret > 0)
            {
                LOG(INFO, "send message successfully! message is %s", con->GetOutbuffer().c_str());
                con->GetOutbuffer().erase(0, ret);
            }
            else if (ret == 0)
            {
                break;
            }
            else
            {
                if (errno == EWOULDBLOCK)
                {
                    // 读阻塞，设置此文件描述符的读事件到内核，等待被唤醒，由reactor亲自执行
                    LOG(INFO, "write block!");
                    con->GetReactor()->SetReadWriteEnable(con->GetSockFd(), true, true);
                    break;
                }
                else if (errno == EINTR)
                {
                    LOG(INFO, "be interrupted by the signal!");
                    continue;
                }
                else
                {
                    LOG(ERROR, "recv error!");
                    HandleExcept(con);
                    return;
                }
            }
        }
    }

    void HandleExcept(Connection* con)
    {
        con->GetReactor()->DelConnection(con->GetSockFd());
    }
private:
    handler_t _process_parse;
};