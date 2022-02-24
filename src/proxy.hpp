#ifndef __PROXY_H__
#define __PROXY_H__

#include "settings.hpp"
#include "cache.hpp"

class Proxy
{
    size_t port;
    size_t curr_id;

public:
    Proxy();
    ~Proxy();
    void proxy_init();
    static void *client_handler(void *req);
};

std::vector<char> server_handler(Request *req);
std::vector<char> recv_response(Request *req, std::string new_request, bool is_revalidation);
std::vector<char> revalidate(Request *req, Response *res);
void not_hit_cache(Request *req);
void get_handler(Request *req);
void post_handler(Request *req);
void connect_handler(Request *req);
void reply_error(Request *req, int err_code);
void deamon_init();

#endif
