#include "MySqlConnectionPool.h"


MySqlConnectionPool::~MySqlConnectionPool()
{
    while(!connections.empty())
    {
        auto conn = connections.front();
        connections.pop();
        conn->close();
    }
}
//获得单例
MySqlConnectionPool* MySqlConnectionPool::GetInstance()
{
    static MySqlConnectionPool instance;
    return &instance;
}

//初始化连接池
void MySqlConnectionPool::init(const std::string& host, const std::string& user, const std::string& password, const std::string& database, int port, int pool_size, int max_connection)
{
    this->host = host;
    this->user = user;
    this->password = password;
    this->database = database;
    this->port = port;
    this->pool_size = pool_size;
    this->max_connection = max_connection;
    for(int i = 0; i < pool_size; ++i)
    {
        addConnection();
    }
}

//从池子里面获得连接
std::shared_ptr<sql::Connection> MySqlConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mtx);
    //while循环，防止虚假唤醒
    while(connections.empty())
    {
        if(tot < max_connection){
            addConnection();
        }else{
            cv.wait(lock);
        }
    }
    auto conn = connections.front();
    connections.pop();
    return conn;
}

//释放连接
void MySqlConnectionPool::ReleaseConnection(std::shared_ptr<sql::Connection> conn)
{
    std::unique_lock<std::mutex> lock(mtx);
    connections.push(conn);
    cv.notify_one();
}

bool MySqlConnectionPool::insert(const std::string& table, const std::map<std::string, std::string>& data)
{
    auto conn = getConnection();
    if(!conn){
        return false;
    }
    try{
    //拼接sql语句
        std::string colums;
        std::string values;
        for(const auto& pair : data) {
            if(!colums.empty()){
                colums += ",";
                values += ",";
            }
            colums += pair.first;
            values += "?";
        }
        //拼接sql语句
        std::string sql = "insert into " + table + "(" + colums + ") values (" + values + ")";

        //创建sql语句，自动释放资源
        std::shared_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));

        //绑定参数
        int idx = 1;
        for(const auto& pair : data) {
            pstmt->setString(idx++, pair.second);
        }

        //执行sql语句
        return pstmt->executeUpdate() > 0;

    }catch(sql::SQLException& e){
        std::cerr << "sql insert error:" << e.what() << std::endl;
        return false;
    }catch(...){
        std::cerr<<"sql insert error: unknown error"<<std::endl;
        return false;
    }
    return true;
}

void MySqlConnectionPool::addConnection()
{
    try{
        sql::Driver* driver = get_driver_instance();
        std::shared_ptr<sql::Connection> conn(driver->connect(host, user, password));
        conn->setSchema(database);
        connections.push(conn);
        ++tot;
    }catch(sql::SQLException& e){
        std::cerr << "Could not create connection :" << e.what() << std::endl;
    }catch(...){
        std::cerr << "Could not create connection: unknown error" << std::endl;
    }
}