#include <iostream>
#include <fstream>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h> 
#include <unistd.h>
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


class Proxy{
    size_t port;
    size_t ID;
public:
    Proxy();
    void proxy_init();
};
