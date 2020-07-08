/**
 * @file
 * @author David Vitez (AKA: Robotic Forest)
 * @copyright All rights reserved © 2020 David Vitez
 */

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

#include "logger.h"
#include "logger_version.h"

/**
 * @brief The simplest logger test. Good for checking if the logger install is working.
 * @details This test logs a number of different messages at every logging level in order to test basic message
 * assembly. A number of fundamental types are all logged from character literals, to string types, and several
 * basic numerical types.
 * @param log
 * — The logger instance to run the test with.
 */
void basicTest(DV::Logger& log) {
    log.debug("Beginning basic logging test.");
    log.info("Testing the logger library. Version:",
            LOGGER_VERSION_MAJOR, '.',
            LOGGER_VERSION_MINOR, '.',
            LOGGER_VERSION_PATCH, '.',
            LOGGER_VERSION_TWEAK
    );

    log.debug("A single string literal argument to the function.");
    log.error("A single string literal argument to the function.");
    log.fatal("A single string literal argument to the function.");
    log.info ("A single string literal argument to the function.");
    log.trace("A single string literal argument to the function.");
    log.warn ("A single string literal argument to the function.");

    log.debug("Many", "string literals", "passed in", "all", "together.");
    log.error("Many", "string literals", "passed in", "all", "together.");
    log.fatal("Many", "string literals", "passed in", "all", "together.");
    log.info ("Many", "string literals", "passed in", "all", "together.");
    log.trace("Many", "string literals", "passed in", "all", "together.");
    log.warn ("Many", "string literals", "passed in", "all", "together.");

    std::string message = "Various types: ";
    log.debug(message, 5, 3.14, 'a', "b c", &message);
    log.error(message, 5, 3.14, 'a', "b c", &message);
    log.fatal(message, 5, 3.14, 'a', "b c", &message);
    log.info (message, 5, 3.14, 'a', "b c", &message);
    log.trace(message, 5, 3.14, 'a', "b c", &message);
    log.warn (message, 5, 3.14, 'a', "b c", &message);
    log.debug("Ending basic logging test.");
}

/**
 * @brief Creates a number of log entries quickly. (Useful in multithreading tests.)
 * @details This test simply outputs a set number of log entries as fast as possible, logging all of the numbers
 * within the range provided. This is expecially good for multithreading tests where one thread could log all the
 * numbers from 1000 to 1999, and another from 2000 to 2999, thus making it clear which thread created which log
 * entries.
 * @param log
 * — The logger instance to be used for testing.
 * @param start
 * — The starting value to log (inclusive). If logging from 0 to 10, the starting value of 0 will be logged.
 * @param end
 * — The ending value which stops the logging loop (exclusive). If logging from 0 to 10, 9 will be the last number
 * logged.
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

/**
 * @brief Creates threads which compete for the logger.
 * @details This test spawns a number of threads which all output log entries as fast as possible to the same logger.
 * If the logger has any thread safety errors in message assembly or character output this will intermittently (but
 * usually) corrupt the internal buffer and/or the output stream.
 * @param log
 * — The logger instance to run the test with.
 */
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

/**
 * @brief The entry point for the logger test application.
 * @details main() is the entry point only for the logger test application and not for the library. The library is
 * compiled separately and is linked to this tester (and the act of linking itself is part of testing the library).
 * @details All testing of the library is done through separate stress test functions. Each function focuses on one
 * aspect of the logger's abilities and they cover everything from basic features up through multithreaded stress tests.
 * @return 0, always.
 */
int main() {

    // Log to the standard character output stream.
    DV::Logger termLog("User Terminal", std::cout);
    basicTest(termLog);

    // Log to a file stream.
    std::ofstream file("file_output.log");
    DV::Logger fileLog("File Output", file);
    basicTest(fileLog);

    // Stress test threads fighting over logging to the terminal and a file.
    threadTest(termLog);
    threadTest(fileLog);

    return 0;
}
