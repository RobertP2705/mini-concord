#include "replica.h"

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>

constexpr int BUFFER_SIZE = 1024;
constexpr int MULTICAST_PORT = 8081;
constexpr const char* MULTICAST_GROUP = "239.0.0.1";

Replica::Replica(int initial_stock) : remaining_stock(initial_stock), curr_sequence_num(0) {
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

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    run(multicast_sock, sender_addr, sender_len);
}

// quantity_buffer format: "sequenceNumber:BUY quantity"
void Replica::process_order(std::string quantity_buffer) {
    int expected_sequence_number = this->curr_sequence_num+1;
    size_t colon_pos = quantity_buffer.find(':');
    size_t space_pos = quantity_buffer.find(' ', colon_pos);

    std::string sequence_number = quantity_buffer.substr(0, colon_pos);
    std::string signal = quantity_buffer.substr(colon_pos + 1, space_pos - colon_pos - 1);
    std::string amount = quantity_buffer.substr(space_pos + 1);

    int seq_num = std::stoi(sequence_number);
    int qty = std::stoi(amount);
    if (seq_num == expected_sequence_number) {
        this->curr_sequence_num = expected_sequence_number; 
        if (qty <= remaining_stock) {
            remaining_stock -= qty;
            std::cout << "Order processed. Remaining stock: " << remaining_stock << std::endl;
        } else {
            std::cout << "Order rejected. Not enough stock. Remaining stock: " << remaining_stock << std::endl;
        }
    } else {
        std::cout << "Order skipped. Invalid sequence number." << std::endl;
    }
}

void Replica::run(int multicast_sock, struct sockaddr_in sender_addr, socklen_t sender_len) {
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(multicast_sock, &readfds);
    while (true) {
        fd_set temp_fds = readfds;
        int activity = select(multicast_sock + 1, &temp_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            std::cerr << "Select error" << std::endl;
            break;
        } else if (activity == 0) {
            std::cout << "No messages received in the last 5 seconds" << std::endl;
        } else {
            if (FD_ISSET(multicast_sock, &temp_fds)) {
                char buffer[BUFFER_SIZE] = {0};
                ssize_t len = recvfrom(multicast_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sender_addr, &sender_len);
                if (len > 0) {
                    std::string quantity_buffer(buffer, len);
                    std::cout << "Replica received: " << quantity_buffer << std::endl;
                    process_order(quantity_buffer);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
