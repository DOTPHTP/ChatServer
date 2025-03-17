#ifndef USERSESSION_H
#define USERSESSION_H
//用户会话类

#include <string>
#include <memory>

class UserSession{
    private:
        std::string username;
        int fd;
        time_t last_heartbeat;
        std::string ip;
        int port;
    public:
        UserSession(std::string username,int fd,std::string ip,int port):username(std::move(username)),fd(fd),ip(std::move(ip)),port(port){
           last_heartbeat = time(nullptr);
        }

         std::string getUsername() const{
              return username;
         }

        int getFd() const{
            return fd;
        }

        std::string getIp() const{
            return ip;
        }

        int getPort() const{
            return port;
        }

        time_t getLastHeartbeat() const{
            return last_heartbeat;
        }

        bool isTimeout() const{
            return time(nullptr) - last_heartbeat > 60;
        }
};



#endif // USERSESSION_H