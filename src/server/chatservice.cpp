#include "chatservice.hpp"
#include "public.hpp"
#include <format>
#include <functional>
#include <muduo/base/Logging.h>
#include <string>

// 获取单例对象的接口函数
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的handler回调操作
ChatService::ChatService()
{
    using namespace std::placeholders;
    msg_handler_map_.emplace(MsgType::LOGIN_MSG,
                             std::bind(&ChatService::login, this, _1, _2, _3));
    msg_handler_map_.emplace(MsgType::REG_MSG,
                             std::bind(&ChatService::reg, this, _1, _2, _3));
}

MsgHandler ChatService::getHandler(MsgType msg_type)
{
    // 记录错误日志，没有对应的回调
    auto it = msg_handler_map_.find(msg_type);
    if (it == msg_handler_map_.end())
    {
        // 返回一个空的处理器
        return [=](const TcpConnectionPtr&, json&, Timestamp)
        {
            LOG_ERROR << std::format("msgid:{} no handler!", static_cast<int>(msg_type));
        };
    }
    else
    {
        return it->second;
    }
}

void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    LOG_INFO << "do login service!";
}
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    std::string name = js["name"];
    std::string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = user_model_.insert(user);
    if(state)
    {
        // 注册成功
        json response;
        response["msgid"] = static_cast<int>(MsgType::REG_MSG_ACK);
        response["id"] = user.getId();
        response["errno"] = 0; // 0表示成功
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = static_cast<int>(MsgType::REG_MSG_ACK);
        response["errno"] = 1;
        conn->send(response.dump());
    }
}