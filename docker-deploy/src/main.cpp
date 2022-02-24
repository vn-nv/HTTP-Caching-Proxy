#include "settings.hpp"
#include "proxy.hpp"

int main()
{
	// deamon_init();
	// while (true)
	// {
		proxy_log.open(LOG_PATH, std::ios_base::out);
		Proxy *my_proxy;
		try
		{
			my_proxy = new Proxy();
			my_proxy->proxy_init();
		}
		catch (std::exception e)
		{
			proxy_log.close();
			delete (my_proxy);
		}
	// }
	return 0;
}