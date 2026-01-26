#include "user_model.hpp"
#include "db/db.h"
#include <format>
#include <mysql/mysql.h>
#include <string>
// TODO：使用预编译sql语句防止sql注入
bool UserModel::insert(User& user)
{
    std::string sql = std::format(
        "insert into User(name,password,state) values('{}','{}','{}')",
        user.getName(), user.getPassword(), user.getState());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    std::string sql = std::format("select * from User where id={}", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(std::stoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::updateState(const User& user)
{
    std::string sql = std::format(
        "update User set state='{}' where id={}",
        user.getState(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }
    return false;
}

void UserModel::resetState()
{
    std::string sql = "update User set state='offline' where state='online'";
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}