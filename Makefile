all : server client

server : sockserver.o
	g++ -o server sockserver.o -lws2_32

sockserver.o : sockserver.cpp
	g++ -c -o sockserver.o sockserver.cpp -lws2_32

client : client.o
	g++ -o client client.o -lws2_32

client.o : client.cpp
	g++ -c -o client.o client.cpp -lws2_32

clean:
	rm *.o server client
