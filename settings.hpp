#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <ctime>
#include <string>
#include <cstring>
#include <unordered_map>
#include <list>
#include <utility>
#include <thread>
#include <mutex>
#include <vector>
#define PORT 12345
#define LOG_PATH "/var/log/erss/proxy.log"
extern std::ofstream proxy_log;
extern int server_fd;


#endif