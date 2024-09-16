#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void send_request(const std::string& server_ip, int server_port, const std::string& request) {
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(client_socket);
        return;
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        close(client_socket);
        return;
    }

    // Send request to server
    if (send(client_socket, request.c_str(), request.size(), 0) < 0) {
        std::cerr << "Error sending request to server" << std::endl;
        close(client_socket);
        return;
    }

    // Receive response from server
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "Server Response: " << buffer << std::endl;
    } else if (bytes_received == 0) {
        std::cerr << "Connection closed by server" << std::endl;
    } else {
        std::cerr << "Error receiving response from server" << std::endl;
    }

    // Close socket
    close(client_socket);
}

int main() {
    std::string server_ip = "127.0.0.1";
    int primary_port = 12345;
    int secondary_port = 12346;

    // Example PUT request
    send_request(server_ip, primary_port, "PUT key1 value1");
    send_request(server_ip, primary_port, "PUT key2 value2");
    send_request(server_ip, primary_port, "PUT key3 value3");
    send_request(server_ip, primary_port, "PUT key4 value4");

    // Example GET request
    send_request(server_ip, primary_port, "GET key1");

    // Example DELETE request
    send_request(server_ip, primary_port, "DELETE key1");

    // Verify replication on secondary server
    send_request(server_ip, secondary_port, "GET key1");
    send_request(server_ip, secondary_port, "GET key2");
    send_request(server_ip, secondary_port, "GET key3");
    send_request(server_ip, secondary_port, "GET key4");

    return 0;
}
