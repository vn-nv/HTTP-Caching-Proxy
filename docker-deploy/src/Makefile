all: link

socket_init.o: socket_init.cpp
	g++ -std=c++11 -pthread -g -c socket_init.cpp -o socket_init.o

http_message.o: http_message.cpp
	g++ -std=c++11 -pthread -g -c http_message.cpp -o http_message.o

proxy.o: proxy.cpp
	g++ -std=c++11 -pthread -g -c proxy.cpp -o proxy.o

main.o: main.cpp
	g++ -std=c++11 -pthread -g -c main.cpp -o main.o

link: socket_init.o http_message.o proxy.o main.o 
	g++ -std=c++11 -pthread -g socket_init.o http_message.o proxy.o main.o -o proxy

clean:
	rm -rf *.o *.~ proxy
	rm -rf *.o *.~ http_message
	rm -rf *.o *.~ socket_init
	rm -rf *.o *.~ main
