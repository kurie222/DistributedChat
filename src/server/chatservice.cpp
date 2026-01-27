#include "chatservice.hpp"
#include "public.hpp"
#include "offline_message_model.hpp"
#include <format>
#include <functional>
#include <muduo/base/Logging.h>
#include <string>
#include <map>

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
    msg_handler_map_.emplace(MsgType::ONE_CHAT_MSG,
                             std::bind(&ChatService::oneChat, this, _1, _2, _3));
    msg_handler_map_.emplace(MsgType::ADD_FRIEND_MSG,
                             std::bind(&ChatService::addFriend, this, _1, _2, _3));
    msg_handler_map_.emplace(MsgType::CREATE_GROUP_MSG,
                             std::bind(&ChatService::createGroup, this, _1, _2, _3));
    msg_handler_map_.emplace(MsgType::ADD_GROUP_MSG,
                             std::bind(&ChatService::addGroup, this, _1, _2, _3));
    msg_handler_map_.emplace(MsgType::GROUP_CHAT_MSG,
                             std::bind(&ChatService::groupChat, this, _1, _2, _3));
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
    int id = js["id"].get<int>();
    std::string password = js["password"];

    User user = user_model_.query(id);
    if (user.getId() != -1 && user.getPassword() == password)
    {
        if (user.getState() == "online")
        {
            // 用户已经登录，不能重复登录
            json response;
            response["msgid"] = static_cast<int>(MsgType::LOGIN_MSG_ACK);
            response["errno"] = 2; // 2表示用户已经登录
            response["errmsg"] = "用户已经登录，不能重复登录";
            conn->send(response.dump());
            return;
        }
        // 登录成功
        {
            std::lock_guard<std::mutex> lock(conn_mutex_);
            user_conn_map_.insert({id, conn});
        }

        user.setState("online");
        user_model_.updateState(user);

        json response;
        response["msgid"] = static_cast<int>(MsgType::LOGIN_MSG_ACK);
        response["id"] = user.getId();
        response["name"] = user.getName();
        response["errno"] = 0; // 0表示成功
        // 查询是否有离线消息
        std::vector<std::string> offline_msgs = offline_message_model_.query(id);
        if (!offline_msgs.empty())
        {
            response["offlinemsg"] = offline_msgs;
            // 删除离线消息
            offline_message_model_.remove(id);
        }
        // 查询好友列表
        std::vector<User> friend_list = friend_model_.query(id);
        if (!friend_list.empty())
        {
            std::vector<json> friends_json;
            for (const auto& friend_user : friend_list)
            {
                json friend_js;
                friend_js["id"] = friend_user.getId();
                friend_js["name"] = friend_user.getName();
                friend_js["state"] = friend_user.getState();
                friends_json.push_back(friend_js);
            }
            response["friends"] = friends_json;
        }

        // 查询群组列表
        std::vector<Group> group_list = group_model_.queryGroups(id);
        if (!group_list.empty())
        {
            std::vector<json> groups_json;
            for (const auto& group : group_list)
            {
                json group_js;
                group_js["id"] = group.getId();
                group_js["name"] = group.getName();
                group_js["desc"] = group.getDesc();
                // 群组成员信息
                std::vector<json> users_json;
                for (const auto& user : group.getUsers())
                {
                    json user_js;
                    user_js["id"] = user.getId();
                    user_js["name"] = user.getName();
                    user_js["role"] = user.getRole();
                    user_js["state"] = user.getState();
                    users_json.push_back(user_js);
                }
                group_js["users"] = users_json;
                groups_json.push_back(group_js);
            }
            response["groups"] = groups_json;
        }

        conn->send(response.dump());
    }
    else
    {
        // 登录失败
        json response;
        response["msgid"] = static_cast<int>(MsgType::LOGIN_MSG_ACK);
        response["errno"] = 1; // 1表示失败
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    std::string name = js["name"];
    std::string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = user_model_.insert(user);
    if (state)
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

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        for (auto it = user_conn_map_.begin(); it != user_conn_map_.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map中删除用户的连接信息
                user = user_model_.query(it->first);
                user_conn_map_.erase(it);
                LOG_INFO << std::format("用户{}异常退出！", user.getId());
                break;
            }
        }
    }
    // 修改用户的状态信息
    if(user.getId() != -1)
    {
        user.setState("offline");
        user_model_.updateState(user);
        return;
    }
}

// 一对一聊天方法
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int to_id = js["to"].get<int>();
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        auto it = user_conn_map_.find(to_id);
        if (it != user_conn_map_.end())
        {
            // 转发消息
            it->second->send(js.dump());
            return;
        }
    }
    // 用户不在线，存储离线消息
    offline_message_model_.insert(to_id, js.dump());
}

void ChatService::resetState()
{
    user_model_.resetState();
}

void ChatService::addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    friend_model_.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    std::string groupname = js["groupname"];
    std::string groupdesc = js["groupdesc"];

    group_model_.createGroup(userid, groupname, groupdesc);
}

void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    group_model_.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["from"].get<int>();
    int groupid = js["groupid"].get<int>();

    // 查询群组成员列表
    std::vector<int> userids = group_model_.queryGroupUsers(userid, groupid);
    {
        std::lock_guard<std::mutex> lock(conn_mutex_);
        for (int id : userids)
        {
            auto it = user_conn_map_.find(id);
            if (it != user_conn_map_.end())
            {
                // 转发消息
                it->second->send(js.dump());
            }
            else
            {
                // 存储离线消息
                offline_message_model_.insert(id, js.dump());
            }
        }
    }
}
