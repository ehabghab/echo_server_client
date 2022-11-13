
all: client server

client:
	g++ -std=c++14 Client_.cpp -o client -lpthread

server:
	g++ -std=c++14 Server_.cpp -o server -lpthread

clean:
	rm client
	rm server

