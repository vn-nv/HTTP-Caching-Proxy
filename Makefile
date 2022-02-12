all: link

main: main.cpp
	g++ -std=c++11 -g -c main.cpp -o main.o

proxy: proxy.cpp
	g++ -std=c++11 -pthread -g -c proxy.cpp -o proxy.o

link: proxy main
	g++ -std=c++11 -pthread -g proxy.o main.o -o proxy

clean:
	rm -rf *.o *.~ proxy
