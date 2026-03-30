#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <sys/socket.h>


class Sequencer {
    private: 
        volatile int sequenceNumber = 0;
    public:
        Sequencer();
};

#endif 
