#ifndef OFFLINE_MESSAGE_MODEL_HPP
#define OFFLINE_MESSAGE_MODEL_HPP

#include <string>
#include <vector>
// 离线消息数据操作类
class OfflineMessageModel
{
public:
    // 存储用户的离线消息
    void insert(int user_id, const std::string& message);
    // 删除用户的离线消息
    void remove(int user_id);
    // 查询用户的离线消息
    std::vector<std::string> query(int user_id);
}; 

#endif