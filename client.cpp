#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

    // Use Railway's specific TCP proxy domain and port
    const char* hostname = "shortline.proxy.rlwy.net";
    const char* port = "15231";
    
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(hostname, port, &hints, &result);
    if (status != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(status) << std::endl;
        return 1;
    }
    
    if (connect(sock, result->ai_addr, result->ai_addrlen) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        freeaddrinfo(result);
        return 1;
    }
    
    freeaddrinfo(result);

    std::cout << "Connected to Railway server via TCP proxy!" << std::endl;

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