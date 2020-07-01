#include <iostream>
#include <fstream>

#include "logger.h"

int main() {

    // Log to the standard character out stream.
    DV::Logger termLog("User Terminal", std::cout);

    termLog.debug("A single string literal argument to the function.");
    termLog.error("A single string literal argument to the function.");
    termLog.fatal("A single string literal argument to the function.");
    termLog.info("A single string literal argument to the function.");
    termLog.trace("A single string literal argument to the function.");
    termLog.warn("A single string literal argument to the function.");

    termLog.debug("Many", "string literals", "passed in", "all", "together.");
    termLog.error("Many", "string literals", "passed in", "all", "together.");
    termLog.fatal("Many", "string literals", "passed in", "all", "together.");
    termLog.info("Many", "string literals", "passed in", "all", "together.");
    termLog.trace("Many", "string literals", "passed in", "all", "together.");
    termLog.warn("Many", "string literals", "passed in", "all", "together.");

    std::string message = "Various types: ";
    termLog.debug(message, 5, 3.14, 'a', "b c", &message);
    termLog.error(message, 5, 3.14, 'a', "b c", &message);
    termLog.fatal(message, 5, 3.14, 'a', "b c", &message);
    termLog.info(message, 5, 3.14, 'a', "b c", &message);
    termLog.trace(message, 5, 3.14, 'a', "b c", &message);
    termLog.warn(message, 5, 3.14, 'a', "b c", &message);

    //Log to a file stream.
    std::ofstream file("file_output.log");
    DV::Logger fileLog("File Output", file);

    fileLog.debug("A single string literal argument to the function.");
    fileLog.error("A single string literal argument to the function.");
    fileLog.fatal("A single string literal argument to the function.");
    fileLog.info("A single string literal argument to the function.");
    fileLog.trace("A single string literal argument to the function.");
    fileLog.warn("A single string literal argument to the function.");

    fileLog.debug("Many", "string literals", "passed in", "all", "together.");
    fileLog.error("Many", "string literals", "passed in", "all", "together.");
    fileLog.fatal("Many", "string literals", "passed in", "all", "together.");
    fileLog.info("Many", "string literals", "passed in", "all", "together.");
    fileLog.trace("Many", "string literals", "passed in", "all", "together.");
    fileLog.warn("Many", "string literals", "passed in", "all", "together.");

    fileLog.debug(message, 5, 3.14, 'a', "b c", &message);
    fileLog.error(message, 5, 3.14, 'a', "b c", &message);
    fileLog.fatal(message, 5, 3.14, 'a', "b c", &message);
    fileLog.info(message, 5, 3.14, 'a', "b c", &message);
    fileLog.trace(message, 5, 3.14, 'a', "b c", &message);
    fileLog.warn(message, 5, 3.14, 'a', "b c", &message);

    return 0;
}
