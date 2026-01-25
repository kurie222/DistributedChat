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