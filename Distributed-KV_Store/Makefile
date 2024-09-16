all: primary_server secondary_server client

primary_server: primary_server.cpp
	g++ -std=c++17 -o primary_server primary_server.cpp

secondary_server: secondary_server.cpp
	g++ -std=c++17 -o secondary_server secondary_server.cpp -pthread

client: client.cpp
	g++ -std=c++17 -o client client.cpp

clean:
	rm -f primary_server secondary_server client
