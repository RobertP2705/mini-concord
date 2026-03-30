#include "sequencer.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;
constexpr int MULTICAST_PORT = 8081;
constexpr const char* MULTICAST_GROUP = "239.0.0.1";

class Sequencer {
    private: 
        volatile int sequenceNumber = 0;
    public:
        Sequencer();
        void run(int server_fd, struct sockaddr_in address, socklen_t addrlen, char buffer[BUFFER_SIZE], int multicast_sock, struct sockaddr_in multicast_addr){
            
        };
};


Sequencer::Sequencer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        close(server_fd);
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to bind socket");
    }

    if(listen(server_fd, 3) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to listen on socket");
    }

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    if (new_socket < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to accept connection");
    }

    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
    if(valread > 0) {
        std::cout << "Sequencer received: " << buffer << std::endl;
        std::string ack = "ACK for TCP";
        send(new_socket, ack.c_str(), ack.size(), 0);

        int multicast_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(multicast_sock < 0) {
            close(new_socket);
            close(server_fd);
            throw std::runtime_error("Failed to create multicast socket");
        }

        struct sockaddr_in multicast_addr;
        memset(&multicast_addr, 0, sizeof(multicast_addr));
        multicast_addr.sin_family = AF_INET;
        multicast_addr.sin_port = htons(MULTICAST_PORT);
        inet_pton(AF_INET, MULTICAST_GROUP, &multicast_addr.sin_addr);

        std::string message = std::to_string(sequenceNumber) + ":" + std::string(buffer);
        size_t message_len = message.length();
        sequenceNumber++;

        if(sendto(multicast_sock, message.c_str(), message_len, 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
            close(multicast_sock);
            close(new_socket);
            close(server_fd);
            throw std::runtime_error("Failed to send multicast message");
        }
    }
}
