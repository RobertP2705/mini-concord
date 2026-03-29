#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <sys/socket.h>


class Sequencer {
    private: 
        volatile int sequenceNumber;
    public:
        Sequencer();
};

#endif 