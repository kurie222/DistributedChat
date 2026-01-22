#ifndef CHATSERVER_HPP
#define CHATSERVER_HPP

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

// 聊天服务器主类
class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop,
               const InetAddress listenAddr,
               const string &nameArg);
    // 启动服务
    void Start();

private:
    // 回调链接相关信息的回调函数
    void OnConnection(const TcpConnectionPtr &);
    // 上报读写时间相关信息的回调函数
    void OnMessage(const TcpConnectionPtr &,
                   Buffer *,
                   Timestamp);
    TcpServer server_; // muduo库实现功能的类对象
    EventLoop *loop_;   // 指向时间循环功能的指针
};

#endif // CHATSERVER_HPP