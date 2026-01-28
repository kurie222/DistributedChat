#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>

void resetHandler(int)
{
    ChatService::instance()->resetState();
    exit(0);
}
int main(int argc,char **argv)
{
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr(argv[1], atoi(argv[2]));
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();
    return 0;
}