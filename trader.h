#ifndef TRADER_H
#define TRADER_H

#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>

constexpr int TRADER_BUFFER_SIZE = 1024;

class Trader {
    private: 
    public:
        Trader();
        void run(int socket, char buffer[TRADER_BUFFER_SIZE]);
};

#endif
