#include "daemon.hpp"

int main()
{
    daemon(false, false);

    while (true)
    {
        sleep(1);
    }
    return 0;
}