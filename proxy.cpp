#include "settings.hpp"
#include "socket_init.hpp"
#include "proxy.hpp"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Proxy::Proxy(){
	this->port = PORT;
	this->curr_id = 0;
}

void Proxy::proxy_init(){
	socket_init();
	while(true){
		char client_ip_addr[INET_ADDRSTRLEN];
		int client_socket_fd;
		pthread_mutex_lock(&mutex);
		build_client(client_ip_addr, client_socket_fd);
		memset(&client_ip_addr, 0, sizeof(client_ip_addr));
		std::time_t seconds = std::time(nullptr);
        std::string request_time = std::string(std::asctime(std::gmtime(&seconds)));
        request_time = request_time.substr(0, request_time.find("\n"));
		std::string client_ip = std::string(client_ip_addr);
		Request* client_request = new Request(this->curr_id, client_socket_fd, client_ip, request_time);
        (this->curr_id)++;
		std::cout<<this->curr_id<<std::endl;
		pthread_mutex_unlock(&mutex);
		pthread_t thread;
		pthread_create(&thread, NULL, client_handler, client_request);
		/*
        std::thread client(&Proxy::client_handler, this, client_request);
        client.join();
		*/
		/*
		hit cache process
		std::thread server(&Proxy::server_handler, this, client_request);
		server.join();
		*/
	}
}


void* Proxy::client_handler(Request* req){
	char buff[1024] = {0};
	int cnt = recv(req->getFd(), buff, 1024, 0);
	if (cnt == -1) {
        perror("recv() failed ");
        return NULL;
    }
	buff[cnt] = '\0';
	std::string buffer = buff;
	std::string method = buffer.substr(0, buffer.find(" "));
	std::string temp1 = buffer.substr(buffer.find(" ") + 1);
	std::string temp2 = buffer.substr(buffer.find("//") + 2);
	std::string hostname = temp2.substr(0, temp2.find("/"));
	std::string query = temp1.substr(temp1.find(hostname) + hostname.length());
	const char *c_host = hostname.c_str();
	struct hostent *host = gethostbyname(c_host);
	pthread_mutex_lock(&mutex);
	proxy_log << req->getIp() << ": " << buffer.substr(0, buffer.find("\r\n")) << " from " << req->getIp() << " @ " <<req->getTime() << std::endl;
	pthread_mutex_unlock(&mutex);
	int server_socket_fd;
	if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		return NULL;
	}
	req->setRq(method, hostname, query, server_socket_fd);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
	if (connect(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect failed");
		return NULL;
	}
	std::cout << "Connected" << std::endl;
	std::string new_request = method + " " + query; 
	std::cout << new_request << std::endl;
	const char *new_buffer = new_request.c_str();

	if (send(server_socket_fd, new_buffer, new_request.length(), 0) < 0)
	{
		perror("send failed");
		return NULL;
	}
	std::cout << "send finished " << std::endl;
	int len = 65503;
	std::vector<char> recv_buffer(len);
	int length = 0;
    do {
        if ((length = recv(server_socket_fd, recv_buffer.data(), recv_buffer.size(), 0)) < 0)
        {
            perror("receive failed");
            return NULL;
        }
        recv_buffer.resize(length);

        // send message back to client
        if (send(req->getFd(), &recv_buffer[0], length, 0) < 0)
        {
            perror("send failed");
            return NULL;
        }
    } while (length > 0);   
	std::cout << "length: " << length << std::endl;

	std::cout << "receive finished\n"
			  << std::endl;
	std::cout << "message to send" << std::endl;
	std::cout << recv_buffer.data();
	// send message back to client

	if (send(req->getFd(), &recv_buffer[0], length, 0) < 0)
	{
		perror("send failed");
		return NULL;
	}
	std::cout << "send finished" <<std::endl;
	close(server_socket_fd);
	return NULL;
}

/*
void Proxy::server_handler(Request* req){
}
*/


void get_request(){
	
}

void post_request(){
	
}