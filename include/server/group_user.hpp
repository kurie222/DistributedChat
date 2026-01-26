#ifndef GROUP_USER_HPP
#define GROUP_USER_HPP

#include "user.hpp"
#include <string>

// GroupUser表的ORM类
class GroupUser : public User
{
public:
    GroupUser(int group_id = -1, int user_id = -1, std::string role = "normal")
        : role_(role) {}

    void setRole(const std::string& role) { role_ = role; }
    std::string getRole() const { return role_; }

private:
    std::string role_;
};

#endif