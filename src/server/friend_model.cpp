#include "friend_model.hpp"
#include "db/db.h"
#include <format>
#include <string>
#include <vector>

void FriendModel::insert(int user_id, int friend_id)
{
    std::string sql = std::format("INSERT INTO Friend (userid, friendid) VALUES ({}, {});", user_id, friend_id);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<User> FriendModel::query(int user_id)
{
    std::vector<User> friend_list;
    std::string sql = std::format("SELECT u.id, u.name, u.state FROM User u "
                                  "INNER JOIN Friend f ON u.id = f.friendid "
                                  "WHERE f.userid = {};",
                                  user_id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(std::stoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friend_list.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return friend_list;
}