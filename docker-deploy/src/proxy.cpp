#include "settings.hpp"
#include "socket_init.hpp"
#include "proxy.hpp"
#include "cache.hpp"

pthread_mutex_t log_mutex;
pthread_mutex_t cache_mutex;
LRUCache *proxy_cache;
std::ofstream proxy_log;

Proxy::Proxy()
{
	this->port = PORT;
	this->curr_id = 0;
}

Proxy::~Proxy()
{
	delete (proxy_cache);
	close(proxy_fd);
}

void Proxy::proxy_init()
{
	socket_init();
	proxy_cache = new LRUCache(CACHE_CAPACITY);
	log_mutex = PTHREAD_MUTEX_INITIALIZER;
	cache_mutex = PTHREAD_MUTEX_INITIALIZER;
	while (true)
	{
		char client_ip_addr[INET_ADDRSTRLEN];
		memset(&client_ip_addr, 0, sizeof(client_ip_addr));
		int client_fd;
		try
		{
			client_fd = build_client(client_ip_addr);
		}
		catch (SocketException e)
		{
			std::cerr << e.what();
			pthread_mutex_lock(&log_mutex);
			proxy_log << "(no-id): ERROR " << e.what() << std::endl;
			pthread_mutex_unlock(&log_mutex);
			continue;
		}
		std::time_t seconds = std::time(nullptr);
		std::string request_time = std::string(std::asctime(std::gmtime(&seconds)));
		request_time = request_time.substr(0, request_time.find("\n"));
		std::string client_ip = std::string(client_ip_addr);
		Request *client_request = new Request(this->curr_id, client_fd, client_ip, request_time);
		(this->curr_id)++;
		pthread_t thread;
		try
		{
			if (pthread_create(&thread, NULL, client_handler, client_request) != 0)
			{
				throw ProxyException("Fail To Create a New Thread");
			}
		}
		catch (ProxyException e)
		{
			std::cerr << e.what();
			pthread_mutex_lock(&log_mutex);
			proxy_log << client_request->getId() << ": ERROR " << e.what() << std::endl;
			pthread_mutex_unlock(&log_mutex);
			close(client_request->getFd());
			delete (client_request);
		}
		catch (SocketException e)
		{
			std::cerr << e.what();
			pthread_mutex_lock(&log_mutex);
			proxy_log << client_request->getId() << ": ERROR " << e.what() << std::endl;
			pthread_mutex_unlock(&log_mutex);
			close(client_request->getFd());
			delete (client_request);
		}
	}
}

void *Proxy::client_handler(void *request)
{
	Request *req = (Request *)request;
	char buff[MAX_BUFFER_SIZE] = {0};
	int cnt = recv(req->getFd(), buff, sizeof(buff), 0);
	if (cnt <= 0)
	{
		reply_error(req, 400);
	}
	buff[cnt] = '\0';
	std::cout << "***********request***********" << std::endl;
	std::cout << buff << std::endl;
	std::cout << "***********request***********" << std::endl
			  << std::endl;
	try
	{
		req->parseRequest(buff);
	}
	catch (ProxyException e)
	{
		reply_error(req, 400);
	}
	pthread_mutex_lock(&log_mutex);
	proxy_log << req->getId() << ": \"" << req->getRequest() << "\" from " << req->getIp() << " @ " << req->getTime() << std::endl;
	pthread_mutex_unlock(&log_mutex);
	try
	{
		if (req->getMethod() == "POST")
		{
			post_handler(req);
		}
		else if (req->getMethod() == "GET")
		{
			get_handler(req);
		}
		else
		{
			connect_handler(req);
		}
	}
	catch (ProxyException e)
	{
		std::cerr << e.what() << std::endl;
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": ERROR " << e.what() << std::endl;
		pthread_mutex_unlock(&log_mutex);
	}
	catch (SocketException e)
	{
		std::cerr << e.what() << std::endl;
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": ERROR " << e.what() << std::endl;
		pthread_mutex_unlock(&log_mutex);
	}
	catch (std::exception e)
	{
		std::cerr << e.what() << std::endl;
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": ERROR " << e.what() << std::endl;
		pthread_mutex_unlock(&log_mutex);
	}
	close(req->getFd());
	pthread_mutex_lock(&log_mutex);
	proxy_log << req->getId() << ": Tunnel closed" << std::endl;
	pthread_mutex_unlock(&log_mutex);
	delete (req);
	pthread_exit(NULL);
	return NULL;
}

std::vector<char> recv_response(Request *req, std::string new_request, bool is_revalidation)
{
	std::vector<char> result;
	int server_fd;
	try
	{
		server_fd = build_server(req->getHost(), false);
		if (server_fd == -1)
		{
			reply_error(req, 400);
		}
	}
	catch (SocketException e)
	{
		std::cerr << e.what() << std::endl;
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": ERROR " << e.what() << std::endl;
		pthread_mutex_unlock(&log_mutex);
		close(server_fd);
		return result;
	}
	const char *new_buffer = new_request.c_str();
	pthread_mutex_lock(&log_mutex);
	proxy_log << req->getId() << ": Requesting \"" << req->getRequest() << "\" from " << req->getHost() << std::endl;
	pthread_mutex_unlock(&log_mutex);
	int length = 0;
	if (send(server_fd, new_buffer, new_request.length(), 0) < 0)
	{
		close(server_fd);
		delete (req);
		pthread_exit(NULL);
	}
	int len = MAX_BUFFER_SIZE;
	std::vector<char> recv_buffer;
	do
	{
		recv_buffer.clear();
		recv_buffer.resize(len);
		if ((length = recv(server_fd, recv_buffer.data(), recv_buffer.size(), 0)) < 0)
		{
			close(server_fd);
			reply_error(req, 502);
		}
		recv_buffer.resize(length);
		if (!is_revalidation && send(req->getFd(), recv_buffer.data(), recv_buffer.size(), 0) < 0)
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": WARNING Socket Send Failed." << std::endl;
			pthread_mutex_unlock(&log_mutex);
			close(server_fd);
			return result;
		}
		result.insert(result.end(), recv_buffer.begin(), recv_buffer.end());
	} while (length > 0);
	close(server_fd);
	return result;
}

std::vector<char> server_handler(Request *req)
{
	std::string new_request = req->getMethod() + " " + req->getQuery();
	return recv_response(req, new_request, false);
}

std::vector<char> revalidate(Request *req, Response *res)
{
	std::string file_proctocal = req->getQuery().substr(0, req->getQuery().find("\r\n"));
	std::string revalidation_request = req->getMethod() + " " + file_proctocal + "\r\n";
	if (res->existEtag())
	{
		revalidation_request += "If-None-Match:" + res->getEtag() + "\r\n";
	}
	if (res->existLastModified())
	{
		revalidation_request += "If-Not-Modified-Since:" + res->getLastModified() + "\r\n";
	}
	if (!res->existEtag() && !res->existLastModified())
	{
		return server_handler(req);
	}
	revalidation_request += "Host:" + req->getHost() + "\r\n\r\n";
	std::cout << "*********revalidation********" << std::endl;
	std::cout << revalidation_request << std::endl;
	std::cout << "*********revalidation********" << std::endl
			  << std::endl;
	return recv_response(req, revalidation_request, true);
}

void reply_error(Request *req, int err_code)
{
	std::string err_res, err_message;
	switch (err_code)
	{
	case 400:
		err_res = "HTTP/1.1 400 Malformed Request";
		err_message = ": ERROR Malformed Request";
	case 502:
		err_res = "HTTP/1.1 502 Bad Gateway";
		err_message = ": ERROR Corrupted Response";
	}
	pthread_mutex_lock(&log_mutex);
	proxy_log << req->getId() << err_message << std::endl;
	proxy_log << req->getId() << ": Responding \"" << err_res << "\"" << std::endl;
	pthread_mutex_unlock(&log_mutex);
	if (send(req->getFd(), (err_message + "\r\n\r\n").c_str(), err_message.length(), 0) < 0)
	{
		std::cerr << "Socket Send Failed";
	}
	pthread_mutex_lock(&log_mutex);
	proxy_log << req->getId() << ": Tunnel closed" << std::endl;
	pthread_mutex_unlock(&log_mutex);
	delete (req);
	pthread_exit(NULL);
}

void not_hit_cache(Request *req)
{
	std::vector<char> result;
	try
	{
		result = server_handler(req);
	}
	catch (std::exception e)
	{
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": ERROR " << e.what() << std::endl;
		pthread_mutex_unlock(&log_mutex);
		return;
	}
	if (result.empty())
	{
		reply_error(req, 502);
	}
	Response *res;
	try
	{
		res = new Response();
		res->parseResponse(result);
	}
	catch (ProxyException e)
	{
		delete (res);
		reply_error(req, 502);
	}
	pthread_mutex_lock(&log_mutex);
	proxy_log << req->getId() << ": Received \"" << res->getResponse() << "\" from " << req->getHost() << std::endl;
	pthread_mutex_unlock(&log_mutex);
	if (res->prohibitStoration())
	{
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": not cacheable because of cache-control: no-store" << std::endl;
		proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\"" << std::endl;
		pthread_mutex_unlock(&log_mutex);
		delete (res);
	}
	else if (res->existCacheControl() && !res->isPublic())
	{
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": not cacheable because of cache-control: private" << std::endl;
		proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\"" << std::endl;
		pthread_mutex_unlock(&log_mutex);
		delete (res);
	}
	else if (res->getStatus() != "200 OK")
	{
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": not cacheable because the status of the response is unexpected" << std::endl;
		proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\"" << std::endl;
		pthread_mutex_unlock(&log_mutex);
		delete (res);
	}
	else
	{
		pthread_mutex_lock(&cache_mutex);
		proxy_cache->put(req->getURL(), res);
		pthread_mutex_unlock(&cache_mutex);
		if (res->existCacheControl())
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": NOTE Cache-Control: " << res->getControlMessage() << std::endl;
			pthread_mutex_unlock(&log_mutex);
		}
		if (res->existEtag())
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": NOTE ETag: " << res->getEtag() << std::endl;
			pthread_mutex_unlock(&log_mutex);
		}
		if (res->existLastModified())
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": NOTE Last-Modified: " << res->getLastModified() << std::endl;
			pthread_mutex_unlock(&log_mutex);
		}
		if (res->requireValidation())
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": cached, but requires re-validation " << std::endl;
			pthread_mutex_unlock(&log_mutex);
		}
		else if (res->existExpiredTime())
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": cached, expires at " << res->getExpiredTime() << std::endl;
			pthread_mutex_unlock(&log_mutex);
		}
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\"" << std::endl;
		pthread_mutex_unlock(&log_mutex);
	}
}

void get_handler(Request *req)
{
	pthread_mutex_lock(&cache_mutex);
	Response *res = proxy_cache->get(req->getURL());
	pthread_mutex_unlock(&cache_mutex);
	if (res != NULL)
	{
		if (res->requireValidation())
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": in cache, requires validation" << std::endl;
			pthread_mutex_unlock(&log_mutex);
			std::vector<char> result = revalidate(req, res);
			// for get method
			Response *revalidation_response = new Response();
			try
			{
				revalidation_response->parseResponse(result);
			}
			catch (ProxyException e)
			{
				delete (res);
				reply_error(req, 502);
			}
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": Received \"" << revalidation_response->getResponse() << "\" from " << req->getHost() << std::endl;
			pthread_mutex_unlock(&log_mutex);
			if (revalidation_response->getStatus() == "304 Not Modified" || revalidation_response->getStatus() == "304 NOT MODIFIED")
			{
				pthread_mutex_lock(&log_mutex);
				proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\"" << std::endl;
				pthread_mutex_unlock(&log_mutex);
				if (send(req->getFd(), res->getContent().data(), res->getContent().size(), 0) < 0)
				{
					throw SocketException("Socket Send Failed");
				}
			}
			else if (revalidation_response->getStatus() == "200 OK")
			{
				pthread_mutex_lock(&log_mutex);
				proxy_cache->put(req->getURL(), revalidation_response);
				pthread_mutex_unlock(&log_mutex);
				pthread_mutex_lock(&log_mutex);
				proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\"" << std::endl;
				pthread_mutex_unlock(&log_mutex);
				if (send(req->getFd(), revalidation_response->getContent().data(), revalidation_response->getContent().size(), 0) < 0)
				{
					throw SocketException("Socket Send Failed");
				}
			}
			else
			{
				throw ProxyException("Unable To Handle the Status");
			}
		}
		else if (res->isExpired())
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": in cache, but expired at " << res->getExpiredTime() << std::endl;
			pthread_mutex_unlock(&log_mutex);
			try
			{
				not_hit_cache(req);
			}
			catch (std::exception e)
			{
				std::cerr << e.what() << std::endl;
				pthread_mutex_lock(&log_mutex);
				proxy_log << req->getId() << ": ERROR " << e.what() << std::endl;
				pthread_mutex_unlock(&log_mutex);
			}
		}
		else
		{
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": in cache, valid" << std::endl;
			pthread_mutex_unlock(&log_mutex);
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": Responding \"" << res->getResponse() << "\"" << std::endl;
			pthread_mutex_unlock(&log_mutex);
			if (send(req->getFd(), res->getContent().data(), res->getContent().size(), 0) < 0)
			{
				throw SocketException("Socket Send Failed");
			}
		}
	}
	else
	{
		pthread_mutex_lock(&log_mutex);
		proxy_log << req->getId() << ": not in cache" << std::endl;
		pthread_mutex_unlock(&log_mutex);
		try
		{
			not_hit_cache(req);
		}
		catch (std::exception e)
		{
			std::cerr << e.what() << std::endl;
			pthread_mutex_lock(&log_mutex);
			proxy_log << req->getId() << ": ERROR " << e.what() << std::endl;
			pthread_mutex_unlock(&log_mutex);
		}
	}
}

void post_handler(Request *req)
{
	std::vector<char> result = server_handler(req);
	if (send(req->getFd(), &result[0], result.size(), 0) < 0)
	{
		throw SocketException("Socket Send Failed");
		return;
	}
	std::string s(result.begin(), result.end());
	pthread_mutex_lock(&log_mutex);
	proxy_log << req->getId() << ": Responding \"" << s.substr(0, s.find("\r\n")) << "\"" << std::endl;
	pthread_mutex_unlock(&log_mutex);
}

void connect_handler(Request *req)
{
	int server_fd = build_server(req->getHost(), true);
	if (server_fd == -1)
	{
		reply_error(req, 400);
	}
	int send_int = send(req->getFd(), "HTTP/1.1 200 OK\r\n\r\n", 19, 0);

	fd_set read_fds, all_fds;
	FD_ZERO(&all_fds);
	FD_SET(req->getFd(), &all_fds);
	FD_SET(server_fd, &all_fds);
	while (true)
	{
		read_fds = all_fds;
		if (select(std::max(req->getFd(), server_fd) + 1, &read_fds, NULL, NULL, NULL) < 0)
		{
			throw SocketException("Socket Select Failed");
			return;
		}

		std::vector<char> buffer(MAX_BUFFER_SIZE);
		int length = 0;
		if (FD_ISSET(req->getFd(), &read_fds))
		{ // client sends message to proxy
			if ((length = recv(req->getFd(), buffer.data(), buffer.size(), 0)) <= 0)
			{
				break;
			}
			else
			{
				if (send(server_fd, buffer.data(), length, 0) <= 0)
				{
					break;
				}
			}
		}

		buffer.clear();
		buffer.resize(MAX_BUFFER_SIZE);
		if (FD_ISSET(server_fd, &read_fds))
		{ // server sends message to proxy
			if ((length = recv(server_fd, buffer.data(), buffer.size(), 0)) <= 0)
			{
				break;
			}
			else
			{
				if (send(req->getFd(), buffer.data(), length, 0) <= 0)
				{
					break;
				}
			}
		}
	}
}

void deamon_init()
{
	pid_t pid;
	// create child process
	if ((pid = fork()))
	{
		std::cout << "fork 1" << std::endl;
		exit(EXIT_SUCCESS);
	}

	// create new session
	if ((pid = setsid()) == -1)
	{
		std::cerr << "setsid error" << std::endl;
		exit(EXIT_FAILURE);
	}

	// point stdin/stdout/stderr at /dev/null
	int fd = open("/dev/null", O_RDWR);
	if (fd == -1)
	{
		std::cerr << "open error" << std::endl;
		exit(EXIT_FAILURE);
	}
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd > 2)
	{
		close(fd);
	}

	// change directory
	if (chdir("/") == -1)
	{
		std::cerr << "chdir error" << std::endl;
		exit(EXIT_FAILURE);
	}

	umask(0);

	if ((pid = fork()))
	{
		std::cout << "fork 2" << std::endl;
		exit(EXIT_SUCCESS);
	}
}