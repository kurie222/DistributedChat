#include "group_model.hpp"
#include "db/db.h"
#include <format>
#include <string>
#include <unordered_map>
#include <vector>

void GroupModel::createGroup(int user_id, const std::string& group_name, const std::string& group_desc)
{
    std::string sql = std::format("INSERT INTO AllGroup (groupname, groupdesc) VALUES ('{}', '{}');", group_name, group_desc);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
        // 获取创建的群组id
        sql = std::format("SELECT ID FROM AllGroup WHERE groupname = '{}' AND groupdesc = '{}' ORDER BY ID DESC LIMIT 1;", group_name, group_desc);
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                int group_id = std::stoi(row[0]);
                mysql_free_result(res);
                // 将创建者加入群组，角色为creator
                addGroup(user_id, group_id, "creator");
            }
        }
    }
}

void GroupModel::addGroup(int user_id, int group_id, const std::string& role)
{
    std::string sql = std::format("INSERT INTO GroupUser (groupid, userid, grouprole) VALUES ({}, {}, '{}');", group_id, user_id, role);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<Group> GroupModel::queryGroups(int user_id)
{
    std::vector<Group> group_list;
    std::unordered_map<int, size_t> pos;

    std::string sql = std::format(
        "SELECT "
        "g.id, g.groupname, g.groupdesc, "
        "u.id, u.name, u.state, gu2.grouprole "
        "FROM AllGroup g "
        "JOIN GroupUser gu_me ON gu_me.groupid = g.id AND gu_me.userid = {} "
        "JOIN GroupUser gu2 ON gu2.groupid = g.id "
        "JOIN User u ON u.id = gu2.userid "
        "ORDER BY g.id;",
        user_id
    );

    MySQL mysql;
    if (!mysql.connect()) return group_list;

    MYSQL_RES* res = mysql.query(sql);
    if (res == nullptr) return group_list;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr)
    {
        int gid = std::stoi(row[0]);

        size_t idx;
        auto it = pos.find(gid);
        if (it == pos.end())
        {
            Group g;
            g.setId(gid);
            g.setName(row[1]);
            g.setDesc(row[2]);
            group_list.push_back(std::move(g));
            idx = group_list.size() - 1;
            pos.emplace(gid, idx);
        }
        else
        {
            idx = it->second;
        }

        GroupUser u;
        u.setId(std::stoi(row[3]));
        u.setName(row[4] ? row[4] : "");
        u.setState(row[5] ? row[5] : "");
        u.setRole(row[6] ? row[6] : "");

        auto users = group_list[idx].getUsers();
        users.push_back(std::move(u));
        group_list[idx].setUsers(std::move(users));
    }

    mysql_free_result(res);
    return group_list;
}

std::vector<int> GroupModel::queryGroupUsers(int user_id, int group_id)
{
    std::vector<int> user_list;
    std::string sql = std::format("SELECT userid FROM GroupUser WHERE groupid = {} AND userid != {};", group_id, user_id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                user_list.push_back(std::stoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return user_list;
}