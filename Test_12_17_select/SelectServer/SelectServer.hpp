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

const static int gsize = 128;
const static int gdefaultval = -1;
class SelectServer
{
private:
    void HandlerAccept()
    {
        InAddr client;
        SockPtr conn_fd = _listen_sock->ServerAccept(&client);
        bool flag = false;

        for (int i = 1; i < gsize; ++i)
        {
            if (fd_array[i] == gdefaultval)
            {
                flag = true;
                fd_array[i] = conn_fd->GetSockFd();
                break;
            }
        }
        if (!flag)
        {
            LOG(ERROR, "fd_array full!");
            close(conn_fd->GetSockFd());
        }
    }
    void HandlerIO(int i)
    {
        char buffer[1024];
        int ret = recv(fd_array[i], buffer, sizeof(buffer) - 1, 0);

        if (ret > 0)
        {
            buffer[ret] = 0;
            LOG(INFO, "client say: %s", buffer);

            std::string header = "HTTP/1.1 200 OK\r\n";
            std::string conn = "Connection: close\r\n";
            std::string content_type = "Content-Type: text/html\r\n";

            std::string message = "<html><body><h1>Hello Linux!</body></h1></html>";

            std::string content_length = "Content-Length: " + std::to_string(message.size()) + "\r\n";

            std::string echo_message = header + content_type + content_length + conn + "\r\n" + message;

            send(fd_array[i], echo_message.c_str(), echo_message.size(), 0);
        }
        else if (ret == 0)
        {
            LOG(INFO, "client quit!");
            close(fd_array[i]);
            fd_array[i] = gdefaultval;
        }
        else
        {
            LOG(ERROR, "receive error!");
            close(fd_array[i]);
            fd_array[i] = gdefaultval;
        }
    }
    void HandlerEvent(fd_set &table)
    {
        for (int i = 0; i < gsize; ++i)
        {
            if (fd_array[i] != gdefaultval)
            {
                if (fd_array[i] == _listen_sock->GetSockFd() && FD_ISSET(fd_array[i], &table))
                {
                    HandlerAccept();
                }
                // 不能直接recv，应为不确定对方是否发送了消息，要让select进行等待
                // ！！一般情况下select需要配合一个数组来进行使用！！
                else if (FD_ISSET(fd_array[i], &table))
                {
                    HandlerIO(i);
                }
            }
        }
    }

public:
    SelectServer(uint16_t port)
        : _port(port), _listen_sock(std::make_unique<TcpSocket>())
    {
        _listen_sock->TcpServerCreateSocket(port);
    }

    void PrintFD()
    {
        for (int i = 0; i < gsize; ++i)
        {
            if (fd_array[i] != gdefaultval)
            {
                std::cout << fd_array[i] << " " ;
            }
        }
        std::cout << std::endl;
    }

    void Init()
    {
        fd_array[0] = _listen_sock->GetSockFd();

        for (int i = 1; i < gsize; ++i)
        {
            fd_array[i] = gdefaultval;
        }
    }

    void Loop()
    {
        while (true)
        {
            fd_set tables;
            FD_ZERO(&tables);
            int max_fd = _listen_sock->GetSockFd();

            // 将所有的现有的需要等待事件的文件描述符插入到 fd_set 中
            for (int i = 0; i < gsize; ++i)
            {
                if (fd_array[i] != gdefaultval)
                {
                    FD_SET(fd_array[i], &tables);
                    if (max_fd < fd_array[i])
                        max_fd = fd_array[i];
                }
            }

            timeval cir_time = {5, 0};

            int ret = select(max_fd + 1, &tables, nullptr, nullptr, &cir_time);
            if (ret > 0)
            {
                LOG(INFO, "有事件就绪！cir_time = %d, %d, ret = %d", cir_time.tv_sec, cir_time.tv_usec, ret);
                HandlerEvent(tables);
                PrintFD();
                continue;
            }
            else if (ret == 0)
            {
                LOG(INFO, "无事件就绪！cir_time = %d, %d, ret = %d", cir_time.tv_sec, cir_time.tv_usec, ret);
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
    {
    }

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listen_sock;

    int fd_array[gsize];
};
