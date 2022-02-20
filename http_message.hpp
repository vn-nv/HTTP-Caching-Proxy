#ifndef __HTTP_MESSAGE_H__
#define __HTTP_MESSAGE_H__

#include "settings.hpp"

class Request{
    size_t id;
    int client_socket_fd;
    std::string request;
    std::string method;
    std::string client_ip;
    std::string time_stamp;
    std::string hostname;
    std::string query;
    std::string url;
public:
    Request(size_t id, int client_socket_fd, std::string client_ip, std::string time_stamp): 
        id(id), client_socket_fd(client_socket_fd), client_ip(client_ip), time_stamp(time_stamp){}
    size_t getId(){return id;}
    int getFd(){return client_socket_fd;}
    std::string getTime(){return time_stamp;}
    std::string getRequest(){return request;}
    std::string getIp(){return client_ip;}
    std::string getURL(){return url;}
    std::string getMethod(){return method;}
    std::string getQuery(){return query;}
    std::string getHost(){return hostname;}
    void setRq(std::string request, std::string method, std::string hostname, std::string query, std::string url){
        this->request = request;
        this->method = method;
        this->hostname = hostname;
        this->query = query;
        this->url = url;}
};

class Response{
    std::string response;
    std::string content;
    std::string status;
    std::string Etag;
    std::string Last_Modified;
    std::string Expired_Time;
    bool cache_public;
    bool cache_control;
    bool no_cache;
    bool no_store;
    bool must_revalidate;
    int max_age = -1;
    time_t expire_time;
    time_t response_time;
    time_t last_modified_time;
public:
    void parseResponse(std::string res);
    void expireTimeConvert(std::string str);
    void responseTimeConvert(std::string str);
    void lastModifiedTimeConvert(std::string str);
    std::string getResponse(){
        return response;
    }
    std::string getStatus(){
        return status;
    }
    std::string getEtag(){
        return Etag;
    }
    std::string getLastModified(){
        return Last_Modified;
    }
    std::string getContent(){
        return content;
    }
    std::string getExpiredTime(){
        return Expired_Time;
    }
    bool prohibitStoration(){
        return no_store;
    }
    bool isPublic(){
        return cache_public;
    }
    bool isExpired(){
        std::time_t curr_time = std::time(nullptr);
        std::asctime(std::gmtime(&curr_time));
        return expire_time && expire_time < curr_time;
    }
    bool existCacheControl(){
        return cache_control;
    }
    bool existEtag(){
        return Etag != "";
    }
    bool existLastModified(){
        return Last_Modified != "";
    }
    bool existExpiredTime(){
        return Expired_Time != "";
    }
    bool requireValidation(){
        std::time_t curr_time = std::time(nullptr);
        std::asctime(std::gmtime(&curr_time));
        std::cout<<curr_time<<std::endl;
        std::cout<<max_age + response_time<<std::endl;
        return no_cache || must_revalidate || max_age == 0 || max_age != -1 && response_time && (max_age + response_time) < curr_time 
        || last_modified_time && response_time && (curr_time - last_modified_time) / 10 < curr_time - response_time;
    }
};

#endif
