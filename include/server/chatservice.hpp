#ifndef CHAT_SERVICE_HPP
#define CHAT_SERVICE_HPP

#include "json.hpp"
#include "muduo/net/TcpConnection.h"
#include "public.hpp"
#include "user_model.hpp"
#include "offline_message_model.hpp"
#include "friend_model.hpp"
#include <functional>
#include <unordered_map>
#include <mutex>
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
    //一对一聊天方法
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(MsgType msg_type);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    // 服务器异常，重置用户状态信息
    void resetState();

private:
    ChatService();
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;
    // 消息id和其对应的业务处理方法
    std::unordered_map<MsgType, MsgHandler> msg_handler_map_;
    // 数据操作类对象
    UserModel user_model_;
    //存储在线用户的连接
    std::unordered_map<int, TcpConnectionPtr> user_conn_map_;
    // 互斥锁，保护user_conn_map_
    std::mutex conn_mutex_;
    // 离线消息数据操作类对象
    OfflineMessageModel offline_message_model_;
    // 好友信息操作类对象
    FriendModel friend_model_;
};

#endif // CHAT_SERVICE_HPP