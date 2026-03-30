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
void Sequencer::run(int new_socket, int multicast_sock) {
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    inet_pton(AF_INET, MULTICAST_GROUP, &multicast_addr.sin_addr);
    multicast_addr.sin_port = htons(MULTICAST_PORT);

    while (true) {
        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
        send(new_socket, "Order received", strlen("Order received"), 0);
        std::cout << "Received order: " << buffer << "; Appending sequence number: " << sequenceNumber << std::endl;
        if(valread > 0) {
            std::lock_guard<std::mutex> lock(mtx);
            sequenceNumber++;
            std::string order(buffer, valread);
            std::string message = std::to_string(sequenceNumber) + ":" + order;
            sendto(multicast_sock, message.c_str(), message.size(), 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr));
        }
    }

    close(new_socket);
}

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


    int multicast_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(multicast_sock < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to create multicast socket");
    }

    while(true){
        int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (new_socket < 0) {
            close(server_fd);
            throw std::runtime_error("Failed to accept connection");
        }
        // starta thread
        std::thread t(&Sequencer::run, this, new_socket, multicast_sock);
        t.detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    close(server_fd);
    close(multicast_sock);
}
