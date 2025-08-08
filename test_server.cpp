#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    
    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::atoi(port_env) : 8080;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    std::cout << "Starting HTTP server on port " << port << std::endl;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "HTTP server listening on port " << port << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "accept error" << std::endl;
            continue;
        }

        std::cout << "HTTP client connected!" << std::endl;

        // Send HTTP response
        const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from Railway!";
        send(clientSocket, response, strlen(response), 0);
        
        close(clientSocket);
    }

    close(serverSocket);
    return 0;
} 