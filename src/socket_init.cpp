#include "socket_init.hpp"

int proxy_fd;

void socket_init()
{
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	// Creating socket file descriptor
	if ((proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw SocketException("Fail to create socket fd");
		return;
	}
	// Forcefully attaching socket to the port 12345
	if (setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				   &opt, sizeof(opt)))
	{
		throw SocketException("Fail to set socket opt");
		return;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	// Forcefully attaching socket to the port 12345
	if (bind(proxy_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		throw SocketException("Fail to bind socket");
		return;
	}
	if (listen(proxy_fd, 3) < 0)
	{
		throw SocketException("Fail to listen socket");
	}
}

int build_client(char *client_ip_addr)
{
	int client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	if ((client_fd = accept(proxy_fd, (struct sockaddr *)&address,
							(socklen_t *)&addrlen)) < 0)
	{
		throw SocketException("Socket Accept Failed");
		return -1;
	}
	if (inet_ntop(AF_INET, &(address.sin_addr), client_ip_addr, INET_ADDRSTRLEN) == NULL)
	{
		throw SocketException("inet_ntop() Failed");
		return -1;
	}
	return client_fd;
}

int build_server(std::string server_host_name, bool is_connect)
{
	int server_fd;
	struct hostent *host;
	struct sockaddr_in server_addr;
	host = gethostbyname(server_host_name.c_str());
	if (host == NULL)
	{
		throw SocketException("Cannot Solve the Domain Name");
		return -1;
	}
	server_addr.sin_family = AF_INET;
	if (is_connect)
	{
		server_addr.sin_port = htons(443);
	}
	else
	{
		server_addr.sin_port = htons(80);
	}
	memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw SocketException("Create Socket Fd Failed");
		return -2;
	}
	if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		throw SocketException("Socket Disconnect");
		return -3;
	}
	return server_fd;
}