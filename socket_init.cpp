#include "socket_init.hpp"

int server_fd;

void socket_init(){
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
	address.sin_port = htons(PORT);
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
}

void build_client(char * client_ip_addr, int &client_socket_fd){
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	std::cout << "enter build" <<std::endl;
	if ((client_socket_fd = accept(server_fd, (struct sockaddr *)&address,
					(socklen_t*)&addrlen))<0)
	{
		perror("accept failed");
		exit(EXIT_FAILURE);
	}
	std::cout << "accepted" << std::endl;
	if (inet_ntop(AF_INET, &(address.sin_addr), client_ip_addr, INET_ADDRSTRLEN) == NULL) {
		perror("inet_ntop() failed");
	}
	std::cout << "leave build" <<std::endl;
}
