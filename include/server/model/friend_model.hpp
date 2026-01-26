#ifndef FRIEND_MODEL_HPP
#define FRIEND_MODEL_HPP

#include "user.hpp"
#include <vector>


// 维护好友信息的接口方法
class FriendModel
{
public:
    // 添加好友
    void insert(int user_id, int friend_id);

    // 获取好友列表
    std::vector<User> query(int user_id);
};

#endif