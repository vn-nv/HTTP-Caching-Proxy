#include "cache.hpp"

void response::parseResponse(std::string res) {
    std::string::size_type pstatus_start = res.find(" ") + 1;
    std::string::size_type pstatus_end = res.find("\r\n");
    status = res.substr(pstatus_start, pstatus_end - pstatus_start + 1);
    std::cout << "status " << status << std::endl;

    std::string::size_type pEtag_start = res.find("ETag: ") + 6;
    if (pEtag_start != 0) {
        std::string::size_type pEtag_end = res.find("\r\n", pEtag_start);
        Etag = res.substr(pEtag_start, pEtag_end - pEtag_start + 1);
        std::cout << "Etag " << Etag << std::endl;
    }

    std::string::size_type pModified_start = res.find("Last-Modified: ") + 14;
    if (pModified_start != 0) {
        std::string::size_type pModified_end = res.find("\r\n", pModified_start);
        Last_Modified = res.substr(pModified_start, pModified_end - pModified_start + 1);
        std::cout << "Last_Modified " << Last_Modified << std::endl;
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

    std::string::size_type pcontent = res.find("\r\n\r\n") + 1;
    content = res.substr(pcontent);
    std::cout << "content " << content << std::endl;
}