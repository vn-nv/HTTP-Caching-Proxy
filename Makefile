all: link

main: main.cpp
	g++ -std=c++11 -g -c main.cpp -o main.o

proxy: proxy.cpp
	g++ -std=c++11 -pthread -g -c proxy.cpp -o proxy.o

cache: cache.cpp
	g++ -std=c++11 -pthread -g -c cache.cpp -o cache.o

socket_init: socket_init.cpp
	g++ -std=c++11 -pthread -g -c socket_init.cpp -o socket_init.o

link: proxy socket_init cache main
	g++ -std=c++11 -pthread -g socket_init.o cache.o proxy.o main.o -o proxy

clean:
	rm -rf *.o *.~ proxy
