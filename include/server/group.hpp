#ifndef GROUP_HPP
#define GROUP_HPP

#include <string>
#include <vector>
#include "group_user.hpp"

//Group表的ORM类
class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
        : id_(id), name_(name), desc_(desc) {}

    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setDesc(const std::string& desc) { desc_ = desc; }
    void setUsers(const std::vector<GroupUser>& users) { users_ = users; }
    int getId() const { return id_; }
    std::string getName() const { return name_; }   
    std::string getDesc() const { return desc_; }
    std::vector<GroupUser> getUsers() const { return users_; }

private:
    int id_;
    std::string name_;
    std::string desc_;
    std::vector<GroupUser> users_;
};


#endif