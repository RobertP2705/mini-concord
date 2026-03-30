#include "trader.h"

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;
constexpr int MULTICAST_PORT = 8081;
constexpr const char* MULTICAST_GROUP = "239.0.0.1";


Trader::Trader() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        close(sock);
        throw std::runtime_error("Invalid address/ Address not supported");
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        throw std::runtime_error("Connection Failed");
    }

    std::string hello = "Hello from client";
    send(sock, hello.c_str(), hello.size(), 0);
    std::cout << "Hello message sent" << std::endl;

    ssize_t valread = read(sock, buffer, BUFFER_SIZE);
    if(valread > 0) {
        std::cout << "Received: " << buffer << std::endl;
    }

    close(sock);

    int multicast_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(multicast_sock < 0) {
        throw std::runtime_error("Failed to create multicast socket");
    }

    struct sockaddr_in multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    multicast_addr.sin_port = htons(MULTICAST_PORT);

    if (bind(multicast_sock, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
        close(multicast_sock);
        throw std::runtime_error("Multicast bind error");
    }

    struct ip_mreq mreq;
    inet_pton(AF_INET, MULTICAST_GROUP, &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(multicast_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        close(multicast_sock);
        throw std::runtime_error("Failed to join multicast group");
    }

    char multicast_buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(multicast_sock, &readfds);

    int activity = select(multicast_sock + 1, &readfds, NULL, NULL, &timeout);
    if (activity > 0 && FD_ISSET(multicast_sock, &readfds)) {
        ssize_t recv_len = recvfrom(multicast_sock, multicast_buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sender_addr, &sender_len);
        if (recv_len > 0) {
            std::cout << "Received multicast: " << multicast_buffer << std::endl;
        } else if (recv_len < 0) {
            std::cerr << "Error receiving multicast" << std::endl;
        }
    } else {
        std::cout << "Multicast receive timeout" << std::endl;
    }
    close(multicast_sock);
}
