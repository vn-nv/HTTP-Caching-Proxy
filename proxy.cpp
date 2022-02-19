#include "settings.hpp"
#include "socket_init.hpp"
#include "proxy.hpp"
#include "cache.hpp"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
LRUCache* proxy_cache;
std::ofstream proxy_log;

Proxy::Proxy(){
	this->port = PORT;
	this->curr_id = 0;
}

void Proxy::proxy_init(){
	socket_init();
	proxy_cache = new LRUCache(CACHE_CAPACITY);
	while(true){
		char client_ip_addr[INET_ADDRSTRLEN];
		int client_socket_fd;
		memset(&client_ip_addr, 0, sizeof(client_ip_addr));			
		build_client(client_ip_addr, client_socket_fd);	
		std::time_t seconds = std::time(nullptr);
        std::string request_time = std::string(std::asctime(std::gmtime(&seconds)));
        request_time = request_time.substr(0, request_time.find("\n"));
		std::string client_ip = std::string(client_ip_addr);
		pthread_mutex_lock(&mutex);
		Request* client_request = new Request(this->curr_id, client_socket_fd, client_ip, request_time);
        (this->curr_id)++;
		pthread_mutex_unlock(&mutex);
		pthread_t thread;
		pthread_create(&thread, NULL, client_handler, client_request);
		//client_handler(client_request);
		//close(client_socket_fd);
	}
}

void* Proxy::client_handler(void* request){
	Request *req = (Request *)request;
	char buff[65536] = {0};
	int cnt = recv(req->getFd(), buff, 1024, 0);
	if (cnt == -1) {
        perror("recv() failed ");
        return NULL;
    }
	buff[cnt] = '\0';
	std::string buffer = buff;
	std::string method = buffer.substr(0, buffer.find(" "));
	std::string temp1 = buffer.substr(buffer.find(" ") + 1);
	std::string url = temp1.substr(0, temp1.find(" "));
	std::string temp2 = buffer.substr(buffer.find("//") + 2);
	std::string hostname = temp2.substr(0, temp2.find("/"));
	std::string query = temp1.substr(temp1.find(hostname) + hostname.length());
	std::string request_line = buffer.substr(0, buffer.find("\r\n"));
	req->setRq(request_line, method, hostname, query, url);
	pthread_mutex_lock(&mutex);
	proxy_log << req->getId() << ": \"" << req->getRequest() << "\" from " << req->getIp() << " @ " <<req->getTime() << std::endl;
	pthread_mutex_unlock(&mutex);
	if(req->getMethod() == "POST"){
		post_handler(req);
	}else if(req->getMethod() == "GET"){
		get_handler(req);
	}else{
		connect_handler(req);
	}
	return NULL;
}

std::vector<char> recv_response(Request* req, std::string new_request){
	std::cout << req->getHost() << std::endl;
	struct hostent *host = gethostbyname((req->getHost()).c_str());

	int server_socket_fd;
	if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	std::cout<<host->h_addr<<std::endl;
	memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
	if (connect(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect failed");
		exit(EXIT_FAILURE);
	}
	std::cout << "Connected" << std::endl;
	std::cout << new_request << std::endl;
	const char *new_buffer = new_request.c_str();
	if (send(server_socket_fd, new_buffer, new_request.length(), 0) < 0)
	{
		perror("send failed");
		exit(EXIT_FAILURE);
	}
	std::cout << "send finished 1" << std::endl;
	int len = 65536;
	std::vector<char> recv_buffer(len);
	std::vector<char> result;
	int length = 0;
    do {
        if ((length = recv(server_socket_fd, recv_buffer.data(), recv_buffer.size(), 0)) < 0)
        {
            perror("receive failed");
            exit(EXIT_FAILURE);
        }
        recv_buffer.resize(length);
		result.insert(result.end(), recv_buffer.begin(), recv_buffer.end());
    } while (length > 0);
	std::cout << "length: " << length << std::endl;
	std::cout << "receive finished" << std::endl;
	std::cout << "message to send" << std::endl;
	close(server_socket_fd);
	return result;
}

std::vector<char> server_handler(Request* req){
	std::string new_request = req->getMethod() + " " + req->getQuery(); 
	pthread_mutex_lock(&mutex);
	proxy_log << req->getId() << ": Requesting \"" <<  req->getRequest() << "\" from " << req->getHost() << std::endl;
	pthread_mutex_unlock(&mutex);
	return recv_response(req, new_request);
}

std::vector<char> revalidate(Request* req, Response* res){
	std::string file_proctocal = req->getQuery().substr(0, req->getQuery().find("\r\n"));
	std::string revalidation_request = req->getMethod() + " " + file_proctocal + "\r\n";
	if(res->existEtag()){
		revalidation_request += "If-None-Match:" + res->getEtag() + "\r\n";
	}
	if(res->existLastModified()){
		revalidation_request += "If-Not-Modified-Since:" + res->getLastModified() + "\r\n";
	}
	if(!res->existEtag() && !res->existLastModified()){
		return server_handler(req);
	}
	revalidation_request += "Host:" + req->getHost() + "\r\n";
	std::cout<<revalidation_request<<std::endl;
	return recv_response(req, revalidation_request);
}

void not_hit_cache(Request* req){
	std::vector<char> result = server_handler(req);
	//convert response to string type
	std::string string_result(result.begin(), result.end());
	//for get method
	Response* server_response = new Response();
	server_response->parseResponse(string_result);
	pthread_mutex_lock(&mutex);
	proxy_log << req->getId() << ": Received \"" <<  server_response->getResponse() << "\" from " << req->getHost() << std::endl;
	pthread_mutex_unlock(&mutex);
	if(server_response->requireResend()){
		//log
		get_handler(req);
		return;
	}else if(server_response->prohibitStoration()){
		pthread_mutex_lock(&mutex);
		proxy_log << req->getId() << ": not cacheable because of cache-control: no-store" << std::endl;
		pthread_mutex_unlock(&mutex);
	}else if(!server_response->isPublic()){
		pthread_mutex_lock(&mutex);
		proxy_log << req->getId() << ": not cacheable because of cache-control: private" << std::endl;
		pthread_mutex_unlock(&mutex);
	}else if(server_response->getStatus() != "200 OK"){ 
		std::cout << server_response->getStatus() <<std::endl;
		pthread_mutex_lock(&mutex);
		proxy_log << req->getId() << ": not cacheable because the status of the response is unexpected" << std::endl;
		pthread_mutex_unlock(&mutex);
	}else{
		proxy_cache->put(req->getURL(), server_response);
		if(server_response->requireValidation()){
			pthread_mutex_lock(&mutex);
			proxy_log << req->getId() << ": cached, but requires re-validation " << std::endl;
			pthread_mutex_unlock(&mutex);
		}else if(server_response->existExpiredTime()){
			pthread_mutex_lock(&mutex);
			proxy_log << req->getId() << ": cached, expires at " << server_response->getExpiredTime() << std::endl;
			pthread_mutex_unlock(&mutex);
		}
	}
	if (send(req->getFd(), &result[0], result.size(), 0) < 0)
	{
		perror("send failed");
	}
	pthread_mutex_lock(&mutex);
	proxy_log << req->getId() << ": Responding \"" << server_response->getResponse() << "\""<< std::endl;
	proxy_log << req->getId() << ": Tunnel closed" <<  std::endl;
	pthread_mutex_unlock(&mutex);
	delete(req);
}

void get_handler(Request* req){
	Response* res = proxy_cache->get(req->getURL());
	if(res != NULL){
		if(res->requireValidation()){
			pthread_mutex_lock(&mutex);
			proxy_log << req->getId() << ": in cache, requires validation" << std::endl;
			pthread_mutex_unlock(&mutex);
			std::vector<char> result = revalidate(req, res);
			//convert response to string type
			std::string string_result(result.begin(), result.end());
			//for get method
			Response* revalidation_response = new Response();
			revalidation_response->parseResponse(string_result);
			if(revalidation_response->getStatus() == "304 Not Modified"){
				if (send(req->getFd(), res->getContent().c_str(), res->getContent().length(), 0) < 0){
					perror("send failed");
				}
				pthread_mutex_lock(&mutex);
				proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\""<< std::endl;
				proxy_log << req->getId() << ": Tunnel closed" <<  std::endl;
				pthread_mutex_unlock(&mutex);
				delete(req);
			}else if(revalidation_response->getStatus() == "200 OK"){
				proxy_cache->put(req->getURL(), revalidation_response);
				if (send(req->getFd(), revalidation_response->getContent().c_str(), revalidation_response->getContent().length(), 0) < 0){
					perror("send failed");
				}
				pthread_mutex_lock(&mutex);
				proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\""<< std::endl;
				proxy_log << req->getId() << ": Tunnel closed" <<  std::endl;
				pthread_mutex_unlock(&mutex);
				delete(req);
			}
		}else if(res->isExpired()){
			pthread_mutex_lock(&mutex);
			proxy_log << req->getId() << ": in cache, but expired at " << res->getExpiredTime() << std::endl;
			pthread_mutex_unlock(&mutex);
			not_hit_cache(req);
		}else{ 
			pthread_mutex_lock(&mutex);
			proxy_log << req->getId() << ": in cache, valid" << std::endl;
			pthread_mutex_unlock(&mutex);
			if (send(req->getFd(), res->getContent().c_str(), res->getContent().length(), 0) < 0){
				perror("send failed");
			}
			pthread_mutex_lock(&mutex);
			proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\""<< std::endl;
			proxy_log << req->getId() << ": Tunnel closed" <<  std::endl;
			pthread_mutex_unlock(&mutex);
			delete(req);
		}
	}else{
		pthread_mutex_lock(&mutex);
		proxy_log << req->getId() << ": not in cache" << std::endl;
		pthread_mutex_unlock(&mutex);
		not_hit_cache(req);
	}
}

void post_handler(Request* req){
	std::vector<char> result = server_handler(req);
	if (send(req->getFd(), &result[0], result.size(), 0) < 0)
    {
		perror("send failed");
    }
	std::string s(result.begin(), result.end());
	pthread_mutex_lock(&mutex);
	proxy_log << req->getId() << ": Responding \"" << s.substr(0, s.find("\r\n")) << "\""<< std::endl;
	proxy_log << req->getId() << ": Tunnel closed" <<  std::endl;
	pthread_mutex_unlock(&mutex);
	delete(req);
}

void connect_handler(Request* req){
}