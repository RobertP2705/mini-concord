class Sequencer {
    private: 
        volatile int sequenceNumber;
    public:
        Sequencer(){
            int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

        };
};
