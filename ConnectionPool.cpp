

#include "ConnectionPool.h"
#include "public.h"


//线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; //staic自带lock和unlock
    return &pool;
}

//连接池构造函数
ConnectionPool::ConnectionPool()
{
    //加载配置项
    if(!loadConfigFile())
    {
        return;
    }

    //创建初始化数量的连接
    for(int i=0;i<_initSize;++i)
    {
        //创建连接添加到队列中
        Connection *p = new Connection();
        p->connect(_ip, _port, _username, _password, _dbname);
        p->refreshAliveTime(); //刷新一下开始空闲的起始时间
        _connectionQue.push(p);
        //记录连接的数量
        _connectionCnt++;
    }

    //启动一个新的线程，作为连接的生产者
    //c++11特性thread
    thread produce(bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach(); //设置为分离线程

    //启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对应的连接回收
    thread scanner(bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach(); //设置为分离线程
}

//加载配置文件
bool ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen("mysql.ini", "r");
    if(pf == nullptr)
    {
        LOG("mysql.ini file is not exist!");
        return false;
    }

    while(!feof(pf))
    {
        char line[1024] = {0};
        fgets(line, 1024, pf);
        string str = line;
        int idx = str.find('=', 0);
        if(idx == -1) //无效的配置项
        {
            continue;
        }
        //换行符
        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx+1, endidx-idx-1);

        if(key == "ip")
        {
            _ip = value;
        }
        else if(key == "port")
        {
            _port = atoi(value.c_str());
        }
        else if(key == "username")
        {
            _username = value;
        }
        else if(key == "password")
        {
            _password = value;
        }
        else if(key == "dbname")
        {
            _dbname = value;
        }
        else if(key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }
        else if(key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }
        else if(key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }
        else if(key == "connectionTimeOut")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

//运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()
{
    for(;;)
    {
        unique_lock<mutex> lock(_queueMutex);
        while(!_connectionQue.empty())
        {
            //阻塞当前线程，直到条件变量被唤醒
            _cv.wait(lock);
        }
        
        //连接数小于最大连接数时，可以创建连接，并将创建的连接放入连接池中
        if(_connectionCnt < _maxSize)
        {
            Connection *p = new Connection();
            p->connect(_ip, _port, _username, _password, _dbname);
            p->refreshAliveTime(); //刷新一下开始空闲的起始时间
            _connectionQue.push(p);
            _connectionCnt++;
        }

        //通知消费者线程可以消费连接了
        _cv.notify_all();
    }
}


shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);

    while(_connectionQue.empty())
    {
        //阻塞当前线程，直到条件变量被唤醒，或到指定时限时长后
        if(std::cv_status::timeout == _cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeout)))
        {
            if(_connectionQue.empty())
            {
                //如果等到上面指定时间结束后队列还是为空，就表示超时了
                LOG("获取空闲连接超时了...获取连接失败！");
                return nullptr;
            }
        }
    }

    /*
    shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于
    调用connection的析构函数，connection就被delete掉了。
    这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue中
    */
    //队列不为空时，则从队列中获取一个连接，自动管理连接的销毁
    shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection *pcon){
        unique_lock<mutex> look(_queueMutex); //为了线程安全，加锁
        pcon->refreshAliveTime(); //刷新一下开始空闲的起始时间
        _connectionQue.push(pcon);
    });
    _connectionQue.pop();
    //消费完连接后，通知生产者线程检查一下，如果队列为空了，赶紧生产连接
    _cv.notify_all();
    return sp;
}

//扫描超过maxIdleTime时间的空闲连接，进行对应的连接回收
void ConnectionPool::scannerConnectionTask()
{
    for(;;)
    {
        //通过sleep模拟定时效果
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
    
        //扫描整个队列，释放多余的连接
        unique_lock<mutex> lock(_queueMutex);
        //如果连接数大于初始时的连接数
        while(_connectionCnt > _initSize)
        {
            Connection *p = _connectionQue.front();
            if(p->getAliveTime() >= (_maxIdleTime * 1000))
            {
                //释放连接，调用Connection的析构函数
                _connectionQue.pop();
                _connectionCnt--; //连接数要减
                delete p;
            }
            else
            {
                break; //队头的连接没有超过_maxIdleTime，其它连接肯定也没有
            }
        }
    }
}