#include "http_message.hpp"

void Response::parseResponse(std::string res) {
    std::cout << "header" << std::endl;
    std::cout << res.substr(0, res.find("\r\n\r\n")) << std::endl;

    std::string::size_type pstatus_start = res.find(" ") + 1;
    std::string::size_type pstatus_end = res.find("\r\n");
    status = res.substr(pstatus_start, pstatus_end - pstatus_start);
    response = res.substr(0, pstatus_end);
    std::cout << "status: " << status << std::endl;

    std::string::size_type pEtag_start = res.find("ETag: ") + 6;
    if (pEtag_start == 5) {
        pEtag_start = res.find("etag: ") + 6;
    }
    if (pEtag_start != 5) {
        std::string::size_type pEtag_end = res.find("\r\n", pEtag_start);
        Etag = res.substr(pEtag_start, pEtag_end - pEtag_start);
        std::cout << "Etag: \"" << Etag << "\"" << std::endl;
    }

    std::string::size_type pModified_start = res.find("Last-Modified: ") + 15;
    if (pModified_start == 14) {
        pModified_start = res.find("last-modified: ") + 15;
    }
    if (pModified_start != 14) {
        std::string::size_type pModified_end = res.find("\r\n", pModified_start);
        Last_Modified = res.substr(pModified_start, pModified_end - pModified_start);
        std::cout << "Last_Modified: " << Last_Modified << std::endl;
        lastModifiedTimeConvert(Last_Modified);
    }

    cache_public = false;
    std::string::size_type is_cache_public = res.find("cache-control");
    if (is_cache_public != -1) {
        cache_public = true;
    }

    cache_public = false;
    std::string::size_type is_cache_public = res.find("public");
    if (is_cache_public != -1) {
        cache_public = true;
    }

    no_cache = false;
    std::string::size_type is_no_cache = res.find("no-cache");
    if (is_no_cache != -1) {
        no_cache = true;
    }

    no_store = false;
    std::string::size_type is_no_store = res.find("no-store");
    if (is_no_store != -1) {
        no_store = true;
    }

    must_revalidate = false;
    std::string::size_type is_must_revalidate = res.find("must-revalidate");
    if (is_must_revalidate != -1) {
        must_revalidate = true;
    }

    max_age = -1;
    std::string::size_type pmax_age_start = res.find("max-age") + 8;
    if (pmax_age_start != 7) {
        std::string::size_type pmax_age_end = res.find("\r\n", pmax_age_start);
        max_age = atoi((res.substr(pmax_age_start, pmax_age_end - pmax_age_start)).c_str());
        std::cout << "max-age: " << max_age << std::endl;
    }

    std::string::size_type pexpire_start = res.find("Expires: ") + 9;
    if (pexpire_start == 8) {
        pexpire_start = res.find("expires: ") + 9;
    }
    if (pexpire_start != 8) {
        std::string::size_type pexpire_end = res.find("\r\n", pexpire_start);
        Expired_Time = res.substr(pexpire_start, pexpire_end - pexpire_start);
        expireTimeConvert(Expired_Time);
    }

    std::string::size_type presponse_start = res.find("Date: ") + 6;
    if (presponse_start == 5) {
        presponse_start = res.find("date: ") + 6;
    }
    if (presponse_start != 5) {
        std::string::size_type presponse_end = res.find("\r\n", presponse_start);
        std::string time = res.substr(presponse_start, presponse_end - presponse_start);
        responseTimeConvert(time);
    }

    content = res;
    //std::cout << "content " << content << std::endl;
}

void Response::expireTimeConvert(std::string str) {
    tm current;
    strptime(str.c_str(), "%a, %d %b %Y %H:%M:%S", &current);
    expire_time = mktime(&current);
}

void Response::responseTimeConvert(std::string str) {
    tm current;
    strptime(str.c_str(), "%a, %d %b %Y %H:%M:%S", &current);
    response_time = mktime(&current);
}

void Response::lastModifiedTimeConvert(std::string str) {
    std::cout << "string " << str << std::endl;
    tm current;
    strptime(str.c_str(), "%a, %d %b %Y %H:%M:%S", &current);
    last_modified_time = mktime(&current);  
    std::cout << "time buffer " << std::endl;
    std::cout << "time minite " << current.tm_min << std::endl;
    std::cout << "time hour " << current.tm_hour << std::endl;
    std::cout << "time day " << current.tm_mday << std::endl;
    std::cout << "time mon " << current.tm_mon << std::endl;
    std::cout << "time year " << current.tm_year <<std::endl;
}