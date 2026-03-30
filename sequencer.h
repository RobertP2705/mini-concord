#ifndef SEQUENCER_H
#define SEQUENCER_H
	
#include <sys/socket.h>
#include <netinet/in.h>


class Sequencer {
    private: 
        volatile int sequenceNumber = 0;
    public:
        Sequencer();
        void run(int new_socket, int multicast_sock);
};

#endif 
