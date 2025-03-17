#ifndef MYSQLCONNECTIONPOOL_H
#define MYSQLCONNECTIONPOOL_H
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <mutex>
#include <queue>
#include <iostream>
#include <condition_variable>
class MySqlConnectionPool
{
    private:
        std::string host;
        std::string user;
        std::string password;
        std::string database;
        int port;
        int max_connection;
        int pool_size;
        int tot = 0;
        std::condition_variable cv;
        std::queue<std::shared_ptr<sql::Connection>> connections;
        std::mutex mtx;
        void addConnection();
        MySqlConnectionPool()=default;
        ~MySqlConnectionPool();
    public:
        //初始化连接池
        void init(
             const std::string& host,
             const std::string& user, 
             const std::string& password,
             const std::string& database, 
             int port, 
             int pool_size,
             int max_connection);
        //获取单例
        static MySqlConnectionPool* GetInstance();
        std::shared_ptr<sql::Connection> getConnection();
        bool insert(const std::string& table,const std::map<std::string,std::string>& data);
        void ReleaseConnection(std::shared_ptr<sql::Connection> conn);
};




#endif // MYSQLCONNECTIONPOOL_H