#ifndef __SOCKET_INIT_H__
#define __SOCKET_INIT_H__

#include "settings.hpp"
void socket_init();
int build_client(char *client_ip_addr);
int build_server(std::string server_host_name, bool is_connect);

#endif