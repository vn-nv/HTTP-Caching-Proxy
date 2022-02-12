#ifndef __PROXY_H__
#define __PROXY_H__

#include "settings.hpp"
#define PORT 12345

class Request{
    size_t id;
    int client_socket_fd;
    std::string client_ip;
    std::string time_stamp;
public:
    Request(size_t id, int client_socket_fd, std::string client_ip, std::string time_stamp): 
        id(id), client_socket_fd(client_socket_fd), client_ip(client_ip), time_stamp(time_stamp){}
    size_t getId(){return id;}
    int getFd(){return client_socket_fd;}
    std::string getIp(){return client_ip;}
    std::string getTime(){return time_stamp;}
};

class Proxy{
    size_t port;
    size_t curr_id;
public:
    Proxy();
    void proxy_init();
    void log_client(Request* req);
};


#endif
