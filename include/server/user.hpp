#ifndef USER_HPP
#define USER_HPP

#include <string>
// 匹配User表的ORM类
class User
{
public:
    User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline")
        : id_(id), name_(name), password_(password), state_(state) {}

    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setPassword(const std::string& password) { password_ = password; }
    void setState(const std::string& state) { state_ = state; }
    int getId() const { return id_; }
    std::string getName() const { return name_; }
    std::string getPassword() const { return password_; }
    std::string getState() const { return state_; }
    
protected:
    int id_;
    std::string name_;
    std::string password_;
    std::string state_;
};

#endif