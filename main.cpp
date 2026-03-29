#include "sequencer.h"
#include "trader.h"

#include <thread>
#include <chrono>
#include <iostream>

int main() {
    try {
        std::cout << "Starting sequencer server thread" << std::endl;

        std::thread sequencerThread([]() {
            Sequencer sequencer;
        });

        // Give sequencer time to start listening before trader connects.
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << "Starting trader client" << std::endl;
        Trader trader;

        sequencerThread.join();

        std::cout << "Test finished" << std::endl;
        return 0;

    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
}
