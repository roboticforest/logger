#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

#include "logger.h"
#include "logger_version.h"

/**
 * @brief The simplest logger test.
 * @details This test is for checking if the logger is working and to see what each logging level looks like.
 * @param log
 * — The logger instance to run the test with.
 */
void basicTest(DV::Logger& log) {
    log.info("Testing the logger library. Version:",
            LOGGER_VERSION_MAJOR, '.',
            LOGGER_VERSION_MINOR, '.',
            LOGGER_VERSION_PATCH, '.',
            LOGGER_VERSION_TWEAK
    );

    log.debug("A single string literal argument to the function.");
    log.error("A single string literal argument to the function.");
    log.fatal("A single string literal argument to the function.");
    log.info("A single string literal argument to the function.");
    log.trace("A single string literal argument to the function.");
    log.warn("A single string literal argument to the function.");

    log.debug("Many", "string literals", "passed in", "all", "together.");
    log.error("Many", "string literals", "passed in", "all", "together.");
    log.fatal("Many", "string literals", "passed in", "all", "together.");
    log.info("Many", "string literals", "passed in", "all", "together.");
    log.trace("Many", "string literals", "passed in", "all", "together.");
    log.warn("Many", "string literals", "passed in", "all", "together.");

    std::string message = "Various types: ";
    log.debug(message, 5, 3.14, 'a', "b c", &message);
    log.error(message, 5, 3.14, 'a', "b c", &message);
    log.fatal(message, 5, 3.14, 'a', "b c", &message);
    log.info(message, 5, 3.14, 'a', "b c", &message);
    log.trace(message, 5, 3.14, 'a', "b c", &message);
    log.warn(message, 5, 3.14, 'a', "b c", &message);
}

/**
 * Used as part of the multithreading stress tests.
 * @details This test simply outputs a set number of log entries as fast as possible.
 * @param log
 * @param start
 * @param end
 */
void logLoop(DV::Logger& log, int start, int end) {
    if (start >= end) {
        log.error("logLoop() Test Failed!");
        log.error(R"(Variable "start" must be < or == variable "end".)");
        return;
    }
    log.debug("Beginning logging loop test.", "Looping", (end - start), "times from", start, "to", (end - 1), '.');
    for (int i = start; i < end; ++i) {
        log.info("Loop iteration:", i);
    }
    log.debug(start, "to", (end - 1), "loop ended.");
}

void threadTest(DV::Logger& log) {
    log.debug("Thread ID:", std::this_thread::get_id());
    log.debug("Spawning 5 other threads.");
    std::thread loop1(logLoop, std::ref(log), 0, 1000);
    log.debug("Started Thread ID:", std::this_thread::get_id(), "--", loop1.get_id());
    std::thread loop2(logLoop, std::ref(log), 1000, 2000);
    log.debug("Started Thread ID:", std::this_thread::get_id(), "--", loop2.get_id());
    std::thread loop3(logLoop, std::ref(log), 2000, 3000);
    log.debug("Started Thread ID:", std::this_thread::get_id(), "--", loop3.get_id());
    std::thread loop4(logLoop, std::ref(log), 3000, 4000);
    log.debug("Started Thread ID:", std::this_thread::get_id(), "--", loop4.get_id());
    std::thread loop5(logLoop, std::ref(log), 4000, 5000);
    log.debug("Started Thread ID:", std::this_thread::get_id(), "--", loop5.get_id());

    // Make sure the threads all spawned correctly and wait for them to finish before returning.
    if (loop1.joinable() &&
            loop2.joinable() &&
            loop3.joinable() &&
            loop4.joinable() &&
            loop5.joinable()) {
        loop1.join();
        loop2.join();
        loop3.join();
        loop4.join();
        loop5.join();
    }
    else {
        log.error("Could not join the 5 sub-threads!");
    }
}

int main() {

    // Log to the standard character output stream.
    DV::Logger termLog("User Terminal", std::cout);
    basicTest(termLog);

    //Log to a file stream.
    std::ofstream file("file_output.log");
    DV::Logger fileLog("File Output", file);
    basicTest(fileLog);

    // Spawn new threads that spawn more threads.
    std::thread termThread(threadTest, std::ref(termLog));
    std::thread fileThread(threadTest, std::ref(fileLog));
    // Wait for those threads to end.
    if (termThread.joinable() && fileThread.joinable()) {
        termThread.join();
        fileThread.join();
    }

    return 0;
}
