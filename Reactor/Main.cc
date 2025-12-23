#include "Reactor.hpp"
#include "Listener.hpp"
#include "NetCal.hpp"
#include "Service.hpp"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        LOG(ERROR, "USAGE METHOD -> %s PORT", argv[0]);
        exit(-1);
    }

    uint16_t port = std::stoi(argv[1]);

    Calculator cal;
    ParsePackage parse(std::bind(&Calculator::Calculate, &cal, std::placeholders::_1));

    HandlerIO ios(std::bind(&ParsePackage::Excute, &parse, std::placeholders::_1));

    
    Listener listener(port);

    std::unique_ptr<Reactor> R = std::make_unique<Reactor>();
    R->SetFunc( std::bind(&HandlerIO::RecvMessage, &ios, std::placeholders::_1), 
                std::bind(&HandlerIO::SendMessage, &ios, std::placeholders::_1),
                std::bind(&HandlerIO::HandleExcept, &ios, std::placeholders::_1),
                std::bind(&Listener::Acceptor, &listener, std::placeholders::_1)
              );
    R->AddConnection(listener.GetListenSockfd(), EPOLLIN | EPOLLET, ListenConnection);
            
    R->Dispatcher();
    
    
    return 0;
}