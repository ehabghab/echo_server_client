
all: client server 

client: Client_.cpp
	g++ -std=c++14 Client_.cpp -o client -lpthread

server: Server_.cpp
	g++ -std=c++14 Server_.cpp -o server -lpthread

clean:
	rm client
	rm server

