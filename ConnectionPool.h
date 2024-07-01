#ifndef __CONNECTIONPOOL_H__
#define __CONNECTIONPOOL_H__

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>
#include <functional>
#include <chrono>
#include "Connection.h"


using std::string;
using std::queue;
using std::mutex;
using std::bind;
using std::thread;
using std::condition_variable;
using std::unique_lock;
using std::atomic_int;
using std::shared_ptr;


class ConnectionPool
{
public:
    //获取连接池对象实例
    static ConnectionPool* getConnectionPool();
    //给外部提供接口，从连接池中获取一个可用的空闲连接
    shared_ptr<Connection> getConnection();

private:
    //单例模式，构造函数私有化
    ConnectionPool();

    //加载配置文件
    bool loadConfigFile();

    //运行在独立的线程中，专门负责生产新连接
    void produceConnectionTask();

    //扫描超过maxIdleTime时间的空闲时间，进行对应的连接回收
    void scannerConnectionTask();

    string _ip; //mysql的host地址
    unsigned short _port; //mysql的端口号
    string _username; //mysql登录用户名
    string _password; //mysql登录密码
    string _dbname; //数据库名称
    int _initSize; //连接池的初始连接量
    int _maxSize; //连接池的最大连接量
    int _maxIdleTime; //连接池最大空闲时间
    int _connectionTimeout; //连接池获取连接的超时时间

    queue<Connection*> _connectionQue; //存储mysql连接的队列
    mutex _queueMutex; //维护连接队列的线程安全互斥锁
    
    atomic_int _connectionCnt; //记录连接所创建的connection连接的总数量

    condition_variable _cv; //设置条件变量，用于连接生产线程和连接消费线程的通信
};

#endif