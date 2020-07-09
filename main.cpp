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
[[maybe_unused]] void basicTest(DV::Logger& log) {
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
[[maybe_unused]] void logLoop(DV::Logger& log, int start, int end) {
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
[[maybe_unused]] void threadTest(DV::Logger& log) {
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
 * @brief Deliberately closes a file stream being used by the logger to see what happens.
 * @details Writing to a closed file stream is an extremely easy way to put a stream into a bad state. This doesn't
 * throw any exceptions, but it does flip the streams bad/error bit and makes it unusable. From the user's perspective
 * all they can see is that the logger isn't printing any more messages. This is something that probably could be
 * recovered from, but only if the logger knew it was working with a file stream (which it doesn't). With regular
 * output streams you can manually clear the error bit and wipe out bad characters. That solution wouldn't work for
 * a closed file stream.
 * @param log
 * — The logger that will unknowingly use a bad output stream.
 */
[[maybe_unused]] void badStreamTest(DV::Logger& log)
{
    log.trace("Entering badStreamTest():");

    std::ofstream file("bad-stream.log");
    DV::Logger fileLog("Bad Stream Logger", file);
    fileLog.info("Closing file...");

    log.debug("file.good:", file.good());
    log.debug("file.isopen:", file.is_open());

    log.debug("Closing file...");
    file.close();

    log.debug("file.good:", file.good());
    log.debug("file.isopen:", file.is_open());

    log.debug("Attempting another write operation.");
    fileLog.error("This message will be in error.");

    log.debug("file.good:", file.good());
    log.debug("file.isopen:", file.is_open());

    log.trace("Exiting badStreamTest().");
}

/**
 * @brief Deliberately deletes an output stream (allocated on the heap) that the logger is using.
 * @details As expected, deleting an object that another object is using causes a segmentation fault. This might be
 * avoidable by having the logger use a smart pointer to a stream instead of a reference.
 * @param log
 * — The logger that will unknowingly use a deleted output stream.
 */
[[maybe_unused]] void deadStreamTest(DV::Logger& log)
{
    log.trace("Entering deadStreamTest():");

    log.debug("Creating file stream object on the heap!");
    auto * file = new std::ofstream("doomed.log");
    DV::Logger fileLog("Doomed Logger", *file);
    fileLog.info("File created.");

    log.debug("file->good:", file->good());
    log.debug("file->isopen:", file->is_open());

    log.debug("Closing file...");
    file->close();
    log.debug("Deleting file stream!");
    delete file;

    log.debug("Attempting another write operation.");
    fileLog.error("This message will be in error.");    // Will this crash?

    log.trace("Exiting deadStreamTest().");
}

/**
 * @brief Test adding extra streams to a logger.
 * @details This test tries to tee a logger multiple times, creating log entries before and after each split.
 */
[[maybe_unused]] void teeStreamTest() {
    DV::Logger log("Multilog", std::cout);

    log.debug("Logger created.");
    log.trace("teeStreamTest() Entered:");
    log.info("Testing output!");
    log.warn("Printing in color. Watch for colors.");

    log.debug("Adding a split/tee to a file.");
    std::ofstream fileA("split-stream-a.log");
    if (!fileA.is_open()) {
        log.error("Could not open file!!!");
        log.error("Aborting test.");
        return;
    }
    log.addSplit(fileA);
    log.debug("Split created.");

    log.info("Testing split (tee'd) output!");

    log.debug("Adding a split/tee to a file.");
    std::ofstream fileB("split-stream-b.log");
    if (!fileB.is_open()) {
        log.error("Could not open file!!!");
        log.error("Aborting test.");
        return;
    }
    log.addSplit(fileB);
    log.debug("Split created.");

    log.info("Testing output with 2 splits!");

    log.debug("Adding a split/tee to a file.");
    std::ofstream fileC("split-stream-c.log");
    if (!fileC.is_open()) {
        log.error("Could not open file!!!");
        log.error("Aborting test.");
        return;
    }
    log.addSplit(fileC);
    log.debug("Split created.");

    log.info("Testing output with 3 splits!");

    log.info("Output is now heading to 4 locations.");
    log.info("Location 1 is the terminal.");
    log.info("Location 2 is split-stream-a.log.");
    log.info("Location 3 is split-stream-b.log.");
    log.info("Location 4 is split-stream-c.log.");

    log.debug("Done testing.");
    log.trace("teeStreamTest() Exiting.");
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
    // DV::Logger termLog("User Terminal", std::cout);
    // basicTest(termLog);

    // Log to a file stream.
    // std::ofstream file("file_output.log");
    // DV::Logger fileLog("File Output", file);
    // basicTest(fileLog);

    // Stress test threads fighting over logging to the terminal and a file.
    // threadTest(termLog);
    // threadTest(fileLog);

    // Test the logger when the stream it's using is in a bad state.
    // badStreamTest(termLog);  // Stream becomes unusable.
    // deadStreamTest(termLog); // Deliberately causes a seg-fault.

    // Test adding additional output stream references to the logger.
    // teeStreamTest();

    DV::Logger multilog("Multi-Log", std::cout);
    std::ofstream fileA("output-a.log");
    std::ofstream fileB("output-b.log");
    std::ofstream fileC("output-c.log");

    multilog.addSplit(fileA);
    multilog.addSplit(fileB);
    multilog.addSplit(fileC);

    threadTest(multilog);

    return 0;
}
