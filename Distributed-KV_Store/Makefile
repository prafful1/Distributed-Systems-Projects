all: primary_server secondary_server client

primary_server: primary_server_original.cpp
	g++ -std=c++17 -o primary_server primary_server_original.cpp

secondary_server: secondary_server_original.cpp
	g++ -std=c++17 -o secondary_server secondary_server_original.cpp -pthread

client: client_original.cpp
	g++ -std=c++17 -o client client_original.cpp

clean:
	rm -f primary_server secondary_server client
