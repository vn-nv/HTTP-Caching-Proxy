#ifndef __SOCKET_INIT_H__
#define __SOCKET_INIT_H__

#include "settings.hpp"
void socket_init();
void build_client(char * client_ip_addr, int &client_socket_fd);

#endif