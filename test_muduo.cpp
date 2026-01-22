#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include <iostream>
#include <string>

// 基于muduo开发服务器程序
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const std::string &nameArg)
        : server_(loop, listenAddr, nameArg), loop_(loop)
    {
        server_.setConnectionCallback([this](const TcpConnectionPtr &ptr)
                                      { this->OnConnection(ptr); });

        server_.setMessageCallback([this](const TcpConnectionPtr &conn,
                                          Buffer *buf,
                                          Timestamp time)
                                   { this->OnMessage(conn, buf, time); });
        server_.setThreadNum(4);
    }
    void Start()
    {
        server_.start();
    }

private:
    void OnConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            std::cout << conn->peerAddress().toIpPort() << " -> "
                      << conn->localAddress().toIpPort() << " state:online " << std::endl;
        }
        else
        {
            std::cout << " state:offline " << std::endl;
            conn->shutdown();
            loop_->quit();
        }
    }
    void OnMessage(const TcpConnectionPtr &conn,
                   Buffer *buffer,
                   Timestamp time)
    {
        std::string buf = buffer->retrieveAllAsString();
        std::cout << buf << std::endl;
        conn->send(buf);
    }
    TcpServer server_;
    EventLoop *loop_;
};

int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "Chat Server");

    server.Start();
    loop.loop();
    return 0;
}