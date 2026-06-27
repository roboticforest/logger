import logger;

#include <iostream>
#include <sstream>
#include <string>

int main()
{
    std::ostringstream captured;
    DV::Logger log("SmokeTest", captured);

    log.info("Version", DV::Logger::Version::string);

    const std::string output = captured.str();
    const bool has_header = output.find("SmokeTest:INFO") != std::string::npos;
    const bool has_version = output.find(std::string(DV::Logger::Version::string)) != std::string::npos;

    if (!has_header || !has_version) {
        std::cerr << "Smoke test output did not contain expected logger content." << std::endl;
        return 1;
    }

    return 0;
}
