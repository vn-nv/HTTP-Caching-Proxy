#include "proxy.h"

Proxy::Proxy(){
	this->port = PORT;
	this->ID = 1;
}

void Proxy::proxy_init(){
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	
	// Forcefully attaching socket to the port 8080
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
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
					(socklen_t*)&addrlen))<0)
	{
		perror("accept failed");
		exit(EXIT_FAILURE);
	}
	char client_ip_addr[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &(address.sin_addr), client_ip_addr, INET_ADDRSTRLEN) == NULL) {
		perror("inet_ntop() failed");
	}
	std::string client_ip = std::string(client_ip_addr);
	valread = read(new_socket, buffer, 1024);
	std::cout<<client_ip<<"    "<<buffer<<std::endl;
}

int main(){
	Proxy my_proxy;
	my_proxy.proxy_init();
	return 0;
}