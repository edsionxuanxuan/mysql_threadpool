#include <iostream>
#include "Connection.h"
#include "ConnectionPool.h"

using std::cout;
using std::endl;

/*
g++ main.cpp Connection.cpp ConnectionPool.cpp  -o main -lmysqlclient -lpthread
*/

int main()
{

    /*
    Connection conn;
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')", "zhang san", 20, "male");
    //写127.0.0.1有问题，写localhost没问题
    bool cn = conn.connect("localhost", 3306, "root", "123", "chat");
    cout << "连接成功：" << cn << endl;
    conn.update(sql);
    */

//    ConnectionPool *cp = ConnectionPool::getConnectionPool();

    clock_t begin = clock();

    //连接池
    ConnectionPool *cp = ConnectionPool::getConnectionPool();

    // for(int i = 0;i<1000; ++i)
    // {
    //     //不使用连接池
    //     // Connection conn;
    //     // char sql[1024] = {0};
    //     // sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')", "zhang san", 20, "male");
    //     // //写127.0.0.1有问题，写localhost没问题
    //     // bool cn = conn.connect("localhost", 3306, "root", "123", "chat");
    //     // cout << "连接成功：" << cn << endl;
    //     // conn.update(sql);

    //     //使用连接池
    //     shared_ptr<Connection> sp = cp->getConnection();
    //     char sql[1024] = {0};
    //     sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')", "zhang san", 20, "male");
    //     sp->update(sql);
    // }

    // clock_t end = clock();

    // cout << "end - begin = " << (end - begin) << "ms" << endl;


    //多线程模拟
    thread t1([&](){
        shared_ptr<Connection> sp = cp->getConnection();
        for(int i=0;i<250;++i){
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')", "zhang san", 20, "male");
            sp->update(sql);
        }
        
    });

    thread t2([&](){
        shared_ptr<Connection> sp = cp->getConnection();
        for(int i=0;i<250;++i){
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')", "zhang san", 20, "male");
            sp->update(sql);
        }
        
    });

    thread t3([&](){
        shared_ptr<Connection> sp = cp->getConnection();
        for(int i=0;i<250;++i){
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')", "zhang san", 20, "male");
            sp->update(sql);
        }
        
    });

    thread t4([&](){
        shared_ptr<Connection> sp = cp->getConnection();
        for(int i=0;i<250;++i){
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', '%d', '%s')", "zhang san", 20, "male");
            sp->update(sql);
        }
        
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    clock_t end = clock();

    cout << "end - begin = " << (end - begin) << "ms" << endl;


    return 0;
}
