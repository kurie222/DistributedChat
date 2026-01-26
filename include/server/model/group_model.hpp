#ifndef GROUP_MODEL_HPP
#define GROUP_MODEL_HPP

#include "group.hpp"
#include <vector>

class GroupModel
{
public:
    // 创建群组
    void createGroup(int user_id, const std::string& group_name, const std::string& group_desc);

    // 加入群组
    void addGroup(int user_id, int group_id, const std::string& role);

    // 查询用户所在的群组信息
    std::vector<Group> queryGroups(int user_id);

    // 查询群组成员id,用于群聊
    std::vector<int> queryGroupUsers(int user_id, int group_id);
};

#endif // GROUP_MODEL_HPP