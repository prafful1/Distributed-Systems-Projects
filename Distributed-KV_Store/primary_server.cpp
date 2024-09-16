#include <iostream>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

std::unordered_map<std::string, std::string> kv_store;

void handle_client(int client_socket, int secondary_socket) {
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        std::cerr << "Error receiving data from client" << std::endl;
        close(client_socket);
        return;
    }
    buffer[bytes_received] = '\0';

    std::string request(buffer);
    std::string response;

    // Parse the request
    std::istringstream iss(request);
    std::string operation, key, value;
    iss >> operation >> key;
    if (operation == "PUT") {
        iss >> value;
        kv_store[key] = value;
        response = "OK";
        // Replicate to secondary server
        if (send(secondary_socket, request.c_str(), request.size(), 0) < 0) {
            std::cerr << "Error replicating PUT operation to secondary server" << std::endl;
        }
    } else if (operation == "GET") {
        if (kv_store.find(key) != kv_store.end()) {
            response = kv_store[key];
        } else {
            response = "NOT_FOUND";
        }
    } else if (operation == "DELETE") {
        kv_store.erase(key);
        response = "OK";
        // Replicate to secondary server
        if (send(secondary_socket, request.c_str(), request.size(), 0) < 0) {
            std::cerr << "Error replicating DELETE operation to secondary server" << std::endl;
        }
    } else {
        response = "INVALID_OPERATION";
    }

    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}

int main() {
    int primary_socket, secondary_socket;
    struct sockaddr_in primary_addr, secondary_addr;

    // Create primary server socket
    primary_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (primary_socket < 0) {
        std::cerr << "Error creating primary server socket" << std::endl;
        return 1;
    }

    primary_addr.sin_family = AF_INET;
    primary_addr.sin_port = htons(12345); // Primary server port
    primary_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(primary_socket, (struct sockaddr *)&primary_addr, sizeof(primary_addr)) < 0) {
        std::cerr << "Error binding primary server socket" << std::endl;
        close(primary_socket);
        return 1;
    }


    if (listen(primary_socket, 5) < 0) {
        std::cerr << "Error listening on primary server socket" << std::endl;
        close(primary_socket);
        return 1;
    }

    // Connect to secondary server
    secondary_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (secondary_socket < 0) {
        std::cerr << "Error creating secondary server socket" << std::endl;
        return 1;
    }

    secondary_addr.sin_family = AF_INET;
    secondary_addr.sin_port = htons(12346); // Secondary server port
    if (inet_pton(AF_INET, "127.0.0.1", &secondary_addr.sin_addr) <= 0) {
        std::cerr << "Invalid secondary server address" << std::endl;
        close(secondary_socket);
        return 1;
    }

    if (connect(secondary_socket, (struct sockaddr *)&secondary_addr, sizeof(secondary_addr)) < 0) {
        std::cerr << "Error connecting to secondary server" << std::endl;
        close(secondary_socket);
        return 1;
    }

    std::cout << "Primary server is running..." << std::endl;

    while (true) {
        int client_socket = accept(primary_socket, NULL, NULL);
        if (client_socket < 0) {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }
        handle_client(client_socket, secondary_socket);
    }

    close(primary_socket);
    close(secondary_socket);
    return 0;
}
