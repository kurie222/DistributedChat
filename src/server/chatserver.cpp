#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <functional>
#include <string>
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress listenAddr,
                       const std::string& nameArg)
    : server_(loop, listenAddr, nameArg), loop_(loop)
{
    // 注册链接回调
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this,
                                            std::placeholders::_1));
    // 注册消息回调
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1,
                                         std::placeholders::_2, std::placeholders::_3));
    // 设置线程数量
    server_.setThreadNum(4);
}

void ChatServer::start() { server_.start(); }

// 回调链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    // 用户断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 上报读写时间相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer,
                           Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    // 反序列化
    json js = json::parse(buf);
    // 目的：解耦网络模块的代码和业务模块的代码
    // 通过messageid来获取handler
    auto handler = ChatService::instance()->getHandler(js["msgid"].get<MsgType>());
    handler(conn, js, time);
}
