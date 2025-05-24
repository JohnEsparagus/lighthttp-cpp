#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

void fail(const std::string &msg) {
    std::cerr << "Error: " << msg << std::endl;
    std::exit(EXIT_FAILURE);
}

int main() {
    std::cout << std::unitbuf;  
    std::cerr << std::unitbuf;

    std::cout << "[+] Server starting up...\n";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) fail("socket() failed");

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        fail("setsockopt() failed");
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(4221);

    if (bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        fail("bind() failed on port 4221");
    }

    if (listen(sockfd, 10) < 0) {
        fail("listen() failed");
    }

    std::cout << "[+] Listening on port 4221...\n";

    while (true) {
        sockaddr_in client {};
        socklen_t client_len = sizeof(client);

        int client_fd = accept(sockfd, reinterpret_cast<sockaddr*>(&client), &client_len);
        if (client_fd < 0) {
            std::cerr << "[!] Failed to accept connection\n";
            continue;
        }

        std::cout << "[*] New connection received\n";

        const std::string response = "HTTP/1.1 200 OK\r\n\r\n";
        send(client_fd, response.c_str(), response.size(), 0);

        close(client_fd);
    }

    close(sockfd);
    return 0;
}

