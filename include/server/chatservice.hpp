#ifndef CHAT_SERVICE_HPP
#define CHAT_SERVICE_HPP

#include "json.hpp"
#include "muduo/net/TcpConnection.h"
#include "public.hpp"
#include <functional>
#include <unordered_map>
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

//处理事件的回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService* instance();
    // 登录业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 注册业务
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(MsgType msg_type);

private:
    ChatService();
    // 消息id和其对应的业务处理方法
    std::unordered_map<MsgType, MsgHandler> msg_handler_map_;
};

#endif // CHAT_SERVICE_HPP