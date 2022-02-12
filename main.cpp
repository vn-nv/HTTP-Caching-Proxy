#include "settings.hpp"
#include "proxy.hpp"

std::ofstream proxy_log;

int main(){
    proxy_log.open(LOG_PATH, std::ios_base::out);
	Proxy my_proxy;
	my_proxy.proxy_init();
    proxy_log.close();
	return 0;
}