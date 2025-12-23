#pragma once
#include <iostream>

#include <fcntl.h>

void SetNonBlock(int sockfd)
{
    int mode = fcntl(sockfd, F_GETFL);

    fcntl(sockfd, F_SETFL, mode | O_NONBLOCK);
}