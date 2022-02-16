#include "settings.h"

class response{
    std::string Etag;
    std::string Last-Modified;
    bool no_chache;
    int max_age = -1;
    date expire_time;
    date response_time;
    std::vetor<char> content;
public:
    void parseResponse(std::string res);

}