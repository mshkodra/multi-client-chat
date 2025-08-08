#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>

int main(int argc, char* argv[]) {
    // Default to localhost
    const char* hostname = "127.0.0.1";
    const char* port = "5432";
    
    // Check command line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "local" || arg == "l") {
            hostname = "127.0.0.1";
            port = "5432";
            std::cout << "Connecting to local server..." << std::endl;
        } else if (arg == "railway" || arg == "r") {
            hostname = "shortline.proxy.rlwy.net";
            port = "15231";
            std::cout << "Connecting to Railway server..." << std::endl;
        } else {
            std::cout << "Usage: " << argv[0] << " [local|railway]" << std::endl;
            std::cout << "  local (or l)  - Connect to localhost:5432" << std::endl;
            std::cout << "  railway (or r) - Connect to Railway deployment" << std::endl;
            std::cout << "  no args       - Connect to localhost:5432 (default)" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Connecting to local server (default)..." << std::endl;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

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
        std::cerr << "Connection Failed to " << hostname << ":" << port << std::endl;
        freeaddrinfo(result);
        return 1;
    }
    
    freeaddrinfo(result);

    std::cout << "Connected to server at " << hostname << ":" << port << std::endl;

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