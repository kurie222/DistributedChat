#include "chatserver.hpp"

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress listenAddr,
                       const string &nameArg)
    : server_(loop, listenAddr, nameArg),
      loop_(loop)
{
    // 注册链接回调
    server_.setConnectionCallback([this](const TcpConnectionPtr &ptr)
                                  { this->OnConnection(ptr); });
    // 注册消息回调
    server_.setMessageCallback([this](const TcpConnectionPtr &ptr,
                                      Buffer *buf,
                                      Timestamp timeStamp)
                               { this->OnMessage(ptr, buf, timeStamp); });
    // 设置线程数量
    server_.setThreadNum(4);
}

void ChatServer::Start()
{
    server_.start();
}

// 回调链接相关信息的回调函数
void OnConnection(const TcpConnectionPtr &)
{
}

// 上报读写时间相关信息的回调函数
void OnMessage(const TcpConnectionPtr &,
               Buffer *,
               Timestamp)
{
}