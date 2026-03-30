#ifndef REPLICA_H
#define REPLICA_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Replica {
    public:
        int remaining_stock;
        int curr_sequence_num;

        Replica(int initial_stock);
        void process_order(std::string quantity_buffer);
        void run(int multicast_sock, struct sockaddr_in sender_addr, socklen_t sender_len);
};

#endif
