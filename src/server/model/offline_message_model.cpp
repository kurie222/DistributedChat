#include "offline_message_model.hpp"
#include "db/db.h"
#include <format>

// 存储用户的离线消息
void OfflineMessageModel::insert(int user_id, const std::string& message)
{
    std::string sql = std::format(
        "insert into OfflineMessage(userid,message) values({},'{}')",
        user_id, message);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除用户的离线消息
void OfflineMessageModel::remove(int user_id)
{
    std::string sql = std::format("delete from OfflineMessage where userid={}", user_id);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
std::vector<std::string> OfflineMessageModel::query(int user_id)
{
    std::string sql = std::format("select message from OfflineMessage where userid={}", user_id);
    std::vector<std::string> vec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            // 遍历结果集合
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.emplace_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}