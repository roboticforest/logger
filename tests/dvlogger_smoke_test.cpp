#include <iostream>
#include <sstream>
#include <string>

#include "logger.h"
#include "version.hpp"

int main()
{
    std::ostringstream captured;
    DV::Logger log("SmokeTest", captured);

    log.info("Version", LOGGER_VERSION_STRING);

    const std::string output = captured.str();
    const bool has_header = output.find("SmokeTest:INFO") != std::string::npos;
    const bool has_version = output.find(LOGGER_VERSION_STRING) != std::string::npos;

    if (!has_header || !has_version) {
        std::cerr << "Smoke test output did not contain expected logger content." << std::endl;
        return 1;
    }

    return 0;
}
