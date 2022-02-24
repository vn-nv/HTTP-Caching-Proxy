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
#include <sys/param.h>
#include <sys/stat.h>
#include <netdb.h>
#include <time.h>
#include <string>
#include <cstring>
#include <unordered_map>
#include <list>
#include <utility>
#include <thread>
#include <mutex>
#include <vector>
#include <fcntl.h>
#include <fstream>
#include "proxy_exception.hpp"
#define PORT 12345
#define CACHE_CAPACITY 100
#define MAX_BUFFER_SIZE 65535
#define LOG_PATH "/var/log/erss/proxy.log"
extern std::ofstream proxy_log;
extern pthread_mutex_t log_mutex;
extern pthread_mutex_t cache_mutex;
extern pthread_mutex_t request_mutex;
extern int proxy_fd;

#endif