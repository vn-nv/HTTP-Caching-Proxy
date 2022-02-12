#include "settings.hpp"
#include "proxy.hpp"

Proxy::Proxy(){
	this->port = PORT;
	this->curr_id = 1;
}

void Proxy::proxy_init(){
	int server_fd, client_socket_fd;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 12345
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	
	// Forcefully attaching socket to the port 12345
	if (bind(server_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}
	while(true){
		memset(&address, 0, sizeof(address));
        addrlen = sizeof(address);
		if ((client_socket_fd = accept(server_fd, (struct sockaddr *)&address,
						(socklen_t*)&addrlen))<0)
		{
			perror("accept failed");
			exit(EXIT_FAILURE);
		}
		char client_ip_addr[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &(address.sin_addr), client_ip_addr, INET_ADDRSTRLEN) == NULL) {
			perror("inet_ntop() failed");
		}
		std::time_t seconds = std::time(nullptr);
        std::string request_time = std::string(std::asctime(std::gmtime(&seconds)));
        request_time = request_time.substr(0, request_time.find("\n"));
		std::string client_ip = std::string(client_ip_addr);
		Request* client_request = new Request(this->curr_id, client_socket_fd, client_ip, request_time);
        (this->curr_id)++;
        std::thread t(&Proxy::log_client, this, client_request);
        t.join();
	}
}

void Proxy::log_client(Request* req){
	char buff[1024] = {0};
	int cnt = recv(req->getFd(), buff, 1024, 0);
	if (cnt == -1) {
        perror("recv() failed ");
        return;
    }
	buff[cnt] = '\0';
	std::string buffer = buff;
	std::string method = buffer.substr(0, buffer.find(" "));
	std::string url = buffer.substr(method.length() + 1, buffer.find(" "));
	std::string protocol = buffer.substr(method.length() + url.length() + 2, buffer.find(" "));
	proxy_log << req->getIp() << ": " << buffer.substr(0, buffer.find("\r\n")) << " from " << req->getIp() << " @ " <<req->getTime() << std::endl;
}

