#ifndef __CACHE_H__
#define __CACHE_H__

#include "settings.hpp"

class response{
    std::string status;
    std::string Etag;
    std::string Last_Modified;
    bool no_cache;
    bool no_store;
    bool must_revalidate;
    //int max_age = -1;
    //date expire_time;
    //date response_time;
    std::string content;
public:
    void parseResponse(std::string res);
};

#endif