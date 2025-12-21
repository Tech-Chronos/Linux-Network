#include "Log.hpp"

#include <sys/epoll.h>

class Multiplex
{
public:
    virtual void AddEvent() = 0;

};

class Epoller : public Multiplex
{
public:
    Epoller()
    {
        epoll_create(128);
    }

    void AddEvent() override
    {
        
    }

};