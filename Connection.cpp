
#include "public.h"
#include "Connection.h"


//初始化数据连接
Connection::Connection()
{
    _conn = mysql_init(nullptr);
}

//释放数据库连接资源
Connection::~Connection()
{
    if(_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

//连接数据库
bool Connection::connect(string ip, unsigned short port, string user, string password, string dbname)
{
    //因为我们接收的参数用的是string类型，而mysql_real_connect中参数类型是const char *，所以需要转换为c字符串
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
    return p != nullptr;
}

//更新操作
bool Connection::update(string sql)
{
    //更新操作 insert delete update
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG("更新失败：" + sql);
        return false;
    }
    return true;
}

//查询操作
MYSQL_RES* Connection::query(string sql)
{
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG("查询失败：" + sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}

