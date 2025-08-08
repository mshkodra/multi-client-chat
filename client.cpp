#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080); // Railway uses port 8080

    // Connect to Railway deployment
    if (inet_pton(AF_INET, "35.212.162.221", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed" << std::endl;
        return 1;
    }

    std::cout << "Connected to Railway server!" << std::endl;

    fd_set readfds;
    int maxfd = (sock > fileno(stdin)) ? sock : fileno(stdin);

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(fileno(stdin), &readfds);

        int activity = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            std::cerr << "select error" << std::endl;
            break;
        }

        // Check for input from server
        if (FD_ISSET(sock, &readfds)) {
            char buffer[1024] = {0};
            int valread = recv(sock, buffer, 1024, 0);
            if (valread > 0) {
                std::cout << "Message from server: " << buffer << std::endl;
            } else if (valread == 0) {
                std::cout << "Server disconnected. Exiting client." << std::endl;
                break;
            } else {
                std::cerr << "recv error" << std::endl;
                break;
            }
        }

        // Check for input from user
        if (FD_ISSET(fileno(stdin), &readfds)) {
            std::string message;
            if (!std::getline(std::cin, message)) {
                // EOF or error
                std::cout << "Input closed. Exiting client." << std::endl;
                break;
            }
            if (!message.empty()) {
                send(sock, message.c_str(), message.length(), 0);
            }
        }
    }

    close(sock);
    return 0;
}