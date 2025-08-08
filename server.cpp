#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <sys/select.h>
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
    
    // Use PORT environment variable (Railway) or default to 8079
    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::atoi(port_env) : 8079;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    std::cout << "Starting server on port " << port << std::endl;

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

    std::cout << "Server listening on port " << port << std::endl;

    std::vector<int> clientSockets;
    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        int maxfd = serverSocket;

        for (int clientSocket : clientSockets) {
            FD_SET(clientSocket, &readfds);
            if (clientSocket > maxfd) {
                maxfd = clientSocket;
            }
        }

        int activity = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            std::cerr << "select error" << std::endl;
            break;
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket < 0) {
                std::cerr << "accept error" << std::endl;
            } else {
                clientSockets.push_back(clientSocket);
                std::cout << "New client connected" << std::endl;
            }
        }

        for (auto it = clientSockets.begin(); it != clientSockets.end(); ) {
            int clientSocket = *it;
            if (FD_ISSET(clientSocket, &readfds)) {
                char buffer[1024] = { 0 };
                int valread = recv(clientSocket, buffer, 1024, 0);
                if (valread > 0) {
                    buffer[valread] = '\0';
                    std::cout << "Message from client: " << buffer << std::endl;
                    ++it;
                } else if (valread == 0) {
                    std::cout << "Client disconnected" << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                } else {
                    std::cerr << "recv error" << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                }
            } else {
                ++it;
            }
        }
    }

    for (int clientSocket : clientSockets) {
        close(clientSocket);
    }
    close(serverSocket);

    return 0;
}