#include "http_message.hpp"

void Request::parseRequest(std::string req)
{
    try
    {
        std::string buffer = req;
        if (buffer.find("\r\n\r\n") == -1)
        {
            throw ProxyException("Malformed Request");
            return;
        }
        method = buffer.substr(0, buffer.find(" "));
        std::string temp1 = buffer.substr(buffer.find(" ") + 1);
        url = temp1.substr(0, temp1.find(" "));
        if (method == "GET" || method == "POST")
        {
            std::string temp2 = buffer.substr(buffer.find("//") + 2);
            hostname = temp2.substr(0, temp2.find("/"));
        }
        else if (method == "CONNECT")
        {
            std::string::size_type pos = url.find(":");
            if (pos != url.npos)
            {
                hostname = url.substr(0, url.find(":"));
            }
            else
            {
                hostname = url.substr(0, url.find(" "));
            }
        }
        else
        {
            throw ProxyException("Malformed Request");
            return;
        }
        query = temp1.substr(temp1.find(hostname) + hostname.length());
        request = buffer.substr(0, buffer.find("\r\n"));
    }
    catch (ProxyException e)
    {
        throw ProxyException("Malformed Request");
    }
    catch (std::exception e)
    {
        throw ProxyException("Malformed Request");
    }
}

void Response::parseResponse(std::vector<char> result)
{
    try
    {
        std::string res(result.begin(), result.end());
        std::cout << "***********response**********" << std::endl;
        content = result;
        if (res.find("\r\n\r\n") == -1)
        {
            throw ProxyException("Corrupted Response");
            return;
        }
        res = res.substr(0, res.find("\r\n\r\n"));
        std::string::size_type pstatus_start = res.find(" ") + 1;
        std::string::size_type pstatus_end = res.find("\r\n");
        status = res.substr(pstatus_start, pstatus_end - pstatus_start);
        response = res.substr(0, pstatus_end);
        std::cout << "status: " << status << std::endl;

        std::string::size_type pEtag_start = res.find("ETag: ") + 6;
        if (pEtag_start == 5)
        {
            pEtag_start = res.find("etag: ") + 6;
        }
        if (pEtag_start != 5)
        {
            std::string::size_type pEtag_end = res.find("\r\n", pEtag_start);
            Etag = res.substr(pEtag_start, pEtag_end - pEtag_start);
            // std::string::size_type weak;
            // if ((weak = Etag.find("W/")) != -1) {
            //     Etag.erase(weak, 2);
            // }
            std::cout << "Etag: " << Etag << std::endl;
        }

        std::string::size_type pModified_start = res.find("Last-Modified: ") + 15;
        if (pModified_start == 14)
        {
            pModified_start = res.find("last-modified: ") + 15;
        }
        if (pModified_start != 14)
        {
            std::string::size_type pModified_end = res.find("\r\n", pModified_start);
            Last_Modified = res.substr(pModified_start, pModified_end - pModified_start);
            std::cout << "Last_Modified: " << Last_Modified << std::endl;
            lastModifiedTimeConvert(Last_Modified);
        }

        cache_control = false;
        std::string::size_type exist_cache_control = res.find("cache-control: ") + 15;
        if (exist_cache_control == 14)
        {
            exist_cache_control = res.find("Cache-Control: ") + 15;
        }
        if (exist_cache_control != 14)
        {
            cache_control = true;
            std::string::size_type pCache_Control_end = res.find("\r\n", exist_cache_control);
            control_message = res.substr(exist_cache_control, pCache_Control_end - exist_cache_control);
            std::cout << "Cache-Control: " << control_message << std::endl;
        }

        cache_public = false;
        std::string::size_type is_cache_public = res.find("public");
        if (is_cache_public != -1)
        {
            cache_public = true;
        }

        no_cache = false;
        std::string::size_type is_no_cache = res.find("no-cache");
        if (is_no_cache != -1)
        {
            no_cache = true;
        }

        no_store = false;
        std::string::size_type is_no_store = res.find("no-store");
        if (is_no_store != -1)
        {
            no_store = true;
        }

        must_revalidate = false;
        std::string::size_type is_must_revalidate = res.find("must-revalidate");
        if (is_must_revalidate != -1)
        {
            must_revalidate = true;
        }

        max_age = -1;
        std::string::size_type pmax_age_start = res.find("max-age") + 8;
        if (pmax_age_start != 7)
        {
            std::string::size_type pmax_age_end = res.find("\r\n", pmax_age_start);
            max_age = atoi((res.substr(pmax_age_start, pmax_age_end - pmax_age_start)).c_str());
            std::cout << "max-age: " << max_age << std::endl;
        }

        std::string::size_type pexpire_start = res.find("Expires: ") + 9;
        if (pexpire_start == 8)
        {
            pexpire_start = res.find("expires: ") + 9;
        }
        if (pexpire_start != 8)
        {
            std::string::size_type pexpire_end = res.find("\r\n", pexpire_start);
            Expired_Time = res.substr(pexpire_start, pexpire_end - pexpire_start);
            expireTimeConvert(Expired_Time);
        }

        std::string::size_type presponse_start = res.find("Date: ") + 6;
        if (presponse_start == 5)
        {
            presponse_start = res.find("date: ") + 6;
        }
        if (presponse_start != 5)
        {
            std::string::size_type presponse_end = res.find("\r\n", presponse_start);
            std::cout << "Date: " << res.substr(presponse_start, presponse_end - presponse_start) << std::endl;
            std::cout << "***********response**********" << std::endl
                      << std::endl;

            std::string time = res.substr(presponse_start, presponse_end - presponse_start);
            responseTimeConvert(time);
        }
    }
    catch (std::exception e)
    {
        std::cerr << e.what() << std::endl;
        throw ProxyException("Corrupted Response");
    }
}

void Response::expireTimeConvert(std::string str)
{
    tm current;
    memset(&current, 0, sizeof(current));
    strptime(str.c_str(), "%a, %d %b %Y %H:%M:%S", &current);
    expire_time = mktime(&current);
}

void Response::responseTimeConvert(std::string str)
{
    tm current;
    memset(&current, 0, sizeof(current));
    strptime(str.c_str(), "%a, %d %b %Y %H:%M:%S", &current);
    response_time = mktime(&current);
    // std::cout << "time buffer " << std::endl;
    // std::cout << "time minite " << current.tm_min << std::endl;
    // std::cout << "time hour " << current.tm_hour << std::endl;
    // std::cout << "time day " << current.tm_mday << std::endl;
    // std::cout << "time mon " << current.tm_mon << std::endl;
    // std::cout << "time year " << current.tm_year <<std::endl;
}

void Response::lastModifiedTimeConvert(std::string str)
{
    tm current;
    memset(&current, 0, sizeof(current));
    strptime(str.c_str(), "%a, %d %b %Y %H:%M:%S", &current);
    last_modified_time = mktime(&current);
    // std::cout << "time buffer " << std::endl;
    // std::cout << "time minite " << current.tm_min << std::endl;
    // std::cout << "time hour " << current.tm_hour << std::endl;
    // std::cout << "time day " << current.tm_mday << std::endl;
    // std::cout << "time mon " << current.tm_mon << std::endl;
    // std::cout << "time year " << current.tm_year <<std::endl;
}