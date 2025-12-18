#include "SelectServer.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        LOG(ERROR, "USAGE METHOD -> %s PORT", argv[0]);
        exit(-1);
    }

    uint16_t port = std::stoi(argv[1]);

    std::unique_ptr<SelectServer> select_server = std::make_unique<SelectServer>(port);
    
    select_server->Init();
    select_server->Loop();
    return 0;
}