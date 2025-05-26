#include <iostream>
#include <format>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#define BUFFER_SIZE 1000
void fail(const std::string &msg) {
    std::cerr << "ERROR: " << msg << std::endl;
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

	char buffer[BUFFER_SIZE];
        int client_fd = accept(sockfd, reinterpret_cast<sockaddr*>(&client), &client_len);
        if (client_fd < 0) {
            std::cerr << "[!] Failed to accept connection\n";
            continue;
        } else {
	    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer),0);
	    if (bytes_read >= 0) {
	        buffer[bytes_read] = '\0';
	        std::cout << "Client send:\n" << buffer << std::endl;
	    } else {
	        fail("Error occured, bytes read was -1...");
	    }
            std::cout << "[*] New connection received\n";
	}
	std::string string_recieved(buffer);

	std::istringstream stream(string_recieved);
	std::string word;
	std::string empty_path = "/";
	stream >> word;          
        if (stream >> word) {        
            std::cout << "Second word: " << word << std::endl;
	}
        else {
            std::cout << "No second word found." << std::endl;
	}
	// we extract the 2nd word cause the average request is like
	// GET /echo/abc HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1....
	// so we can extract /echo/abc and then do wbat is necessary
	
	if (empty_path==word) {
            const std::string response = "HTTP/1.1 200 OK\r\n\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
	} else if (word.substr(0,6)=="/echo/") {
            std::string sub = word.substr(5);
	    size_t response_size = sub.size();
	    const std::string response = std::format("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {}\r\n\r\n{}", response_size, sub);

            send(client_fd, response.c_str(), response.size(), 0);
	} else {
	    const std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
	}
	close(client_fd);
    }

    close(sockfd);
    return 0;
}

