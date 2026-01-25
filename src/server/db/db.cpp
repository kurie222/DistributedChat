#include "db.h"

#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <string>
using namespace muduo;

// 数据库配置信息
static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "123456";
static std::string db_name = "chat";

MySQL::MySQL()
{
    conn_ = mysql_init(nullptr);
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (conn_ != nullptr)
        mysql_close(conn_);
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL* p = mysql_real_connect(conn_, server.c_str(), user.c_str(),
                                  password.c_str(), db_name.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        mysql_query(conn_, "set names gbk");
        LOG_INFO << "connect mysql database success!";
    }
    else 
    {
        LOG_INFO << "connect mysql database fail! errno=" << mysql_errno(conn_)
                 << ", error=" << mysql_error(conn_);
    }
    return p;
}
// 更新操作
bool MySQL::update(std::string sql)
{
    if (mysql_query(conn_, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql
                 << "更新失败! errno=" << mysql_errno(conn_)
                 << ", error=" << mysql_error(conn_);
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES* MySQL::query(std::string sql)
{
    if (mysql_query(conn_, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql
                 << "查询失败! errno=" << mysql_errno(conn_)
                 << ", error=" << mysql_error(conn_);
        return nullptr;
    }
    return mysql_use_result(conn_);
}

// 获取连接
MYSQL* MySQL::getConnection()
{
    return conn_;
}