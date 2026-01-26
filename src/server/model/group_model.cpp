#include "group_model.hpp"
#include "db/db.h"
#include <format>
#include <string>

void GroupModel::createGroup(int user_id, const std::string& group_name, const std::string& group_desc)
{
    std::string sql = std::format("INSERT INTO AllGroup (groupname, groupdesc) VALUES ('{}', '{}');", group_name, group_desc);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
        // 获取创建的群组id
        sql = "SELECT LAST_INSERT_ID();";
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
    std::string sql = std::format("SELECT g.id, g.groupname, g.groupdesc FROM AllGroup g "
                                  "INNER JOIN GroupUser gu ON g.id = gu.groupid "
                                  "WHERE gu.userid = {};",
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
                Group group;
                group.setId(std::stoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                group_list.push_back(group);
            }
            mysql_free_result(res);
        }
    }
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