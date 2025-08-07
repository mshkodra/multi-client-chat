#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <sys/select.h>

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    listen(serverSocket, 5);


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
            std::cout << "select error" << std::endl;
            continue;
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            clientSockets.push_back(clientSocket);
            std::cout << "New client connected" << std::endl;
        }

        for (int clientSocket : clientSockets) {
            if (FD_ISSET(clientSocket, &readfds)) {
                char buffer[1024] = { 0 };
                int valread = recv(clientSocket, buffer, 1024, 0);

                std::cout << "Message from client: " << buffer << std::endl;
            }
        }
    }

    close(serverSocket);

    return 0;
}