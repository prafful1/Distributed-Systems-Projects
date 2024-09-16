#include <iostream>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

std::unordered_map<std::string, std::string> kv_store;

void handle_primary(int primary_socket) {
    char buffer[1024];
    int bytes_received = recv(primary_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        std::cerr << "Error receiving data from primary server" << std::endl;
        close(primary_socket);
        return;
    }
    buffer[bytes_received] = '\0';

    std::string request(buffer);

    // Parse the request
    std::istringstream iss(request);
    std::string operation, key, value;
    iss >> operation >> key;
    if (operation == "PUT") {
        iss >> value;
        kv_store[key] = value;
    } else if (operation == "DELETE") {
        kv_store.erase(key);
    }
}

void handle_client(int client_socket) {
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
    std::string operation, key;
    iss >> operation >> key;
    if (operation == "GET") {
        if (kv_store.find(key) != kv_store.end()) {
            response = kv_store[key];
        } else {
            response = "NOT_FOUND";
        }
    } else {
        response = "INVALID_OPERATION";
    }

    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}

void primary_server_handler(int primary_socket) {
    while (true) {
        handle_primary(primary_socket);
    }
}

int main() {
    int secondary_socket, primary_socket, client_socket;
    struct sockaddr_in secondary_addr, primary_addr, client_addr;
    socklen_t primary_addr_len = sizeof(primary_addr);
    socklen_t client_addr_len = sizeof(client_addr);

    // Create secondary server socket
    secondary_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (secondary_socket < 0) {
        std::cerr << "Error creating secondary server socket" << std::endl;
        return 1;
    }

    secondary_addr.sin_family = AF_INET;
    secondary_addr.sin_port = htons(12346); // Secondary server port
    secondary_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(secondary_socket, (struct sockaddr *)&secondary_addr, sizeof(secondary_addr)) < 0) {
        std::cerr << "Error binding secondary server socket" << std::endl;
        close(secondary_socket);
        return 1;
    }

    if (listen(secondary_socket, 5) < 0) {
        std::cerr << "Error listening on secondary server socket" << std::endl;
        close(secondary_socket);
        return 1;
    }

    std::cout << "Secondary server is running..." << std::endl;

    primary_addr.sin_family = AF_INET;
    primary_addr.sin_port = htons(12345); // Primary server port
    primary_addr.sin_addr.s_addr = INADDR_ANY;

    // Accept connection from primary server
    primary_socket = accept(secondary_socket, (struct sockaddr *)&primary_addr, &primary_addr_len);
    if (primary_socket < 0) {
        std::cerr << "Error accepting connection from primary server" << std::endl;
        close(secondary_socket);
        return 1;
    }

    // Create a separate thread to handle primary server requests
    std::thread primary_thread([&primary_socket]() {
        primary_server_handler(primary_socket);
    });

    // Handle client connections
    while (true) {
        client_socket = accept(secondary_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }
        handle_client(client_socket);
    }

    primary_thread.join();
    close(secondary_socket);
    close(primary_socket);
    return 0;
}
