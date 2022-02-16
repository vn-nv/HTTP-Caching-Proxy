#ifndef __PROXY_H__
#define __PROXY_H__

#include "settings.hpp"


class Request{
    size_t id;
    int client_socket_fd;
    int server_socket_fd;
    std::string method;
    std::string client_ip;
    std::string time_stamp;
    std::string hostname;
    std::string query;
public:
    Request(size_t id, int client_socket_fd, std::string client_ip, std::string time_stamp): 
        id(id), client_socket_fd(client_socket_fd), client_ip(client_ip), time_stamp(time_stamp){}
    size_t getId(){return id;}
    int getFd(){return client_socket_fd;}
    std::string getIp(){return client_ip;}
    std::string getTime(){return time_stamp;}
    void setRq(std::string method, std::string hostname, std::string query, int server_socket_fd){
        this->method = method;
        this->hostname = hostname;
        this->query = query;
        this->server_socket_fd = server_socket_fd;}
};

class Proxy{
    size_t port;
    size_t curr_id;
public:
    Proxy();
    void proxy_init();
    void *client_handler(Request* req);
    //void server_handler(Request* req);
};


#endif
