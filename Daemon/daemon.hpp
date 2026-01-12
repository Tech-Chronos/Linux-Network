#pragma once

#include <iostream>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

std::string default_dir = "/";
std::string default_fd = "/dev/null";

void daemon(bool ifchdir, bool ifclose)
{
    // 1. 忽略信号
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    // 2. 不能让当前进程组的组长创建新的会话
    if (fork() > 0)
        exit(0);

    // 3. 子进程创建新的会话
    if (setsid() < 0)
    {
        std::cerr << "setsid error!" << std::endl;
        exit(-1);
    }

    // 4. 看是否需要改变进程的工作目录
    if (ifchdir)
    {
        chdir(default_dir.c_str());
    }

    // 5. 看是否需要关闭当前的文件描述符
    if (ifclose)
    {
        ::close(0);
        ::close(1);
        ::close(2);
    }
    else
    {
        int fd = open(default_fd.c_str(), O_RDWR);
        if (fd < 0)
        {
            std::cerr << "open error!" << std::endl;
            exit(-1);
        }
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);

        ::close(fd);
    }
}