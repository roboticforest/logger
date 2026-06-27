import logger;

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include "../test-support.hpp"

namespace {

void log_loop(DV::Logger& log, int start, int end)
{
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

}  // namespace

int main()
{
    std::ostringstream captured;
    DV::Logger log("ManualStress", captured);

    std::thread loop1(log_loop, std::ref(log), 0, 1000);
    std::thread loop2(log_loop, std::ref(log), 1000, 2000);
    std::thread loop3(log_loop, std::ref(log), 2000, 3000);
    std::thread loop4(log_loop, std::ref(log), 3000, 4000);
    std::thread loop5(log_loop, std::ref(log), 4000, 5000);

    loop1.join();
    loop2.join();
    loop3.join();
    loop4.join();
    loop5.join();

    const std::string output = captured.str();
    if (output.empty()) {
        std::cerr << "Manual stress test produced no output." << std::endl;
        return 1;
    }
    return 0;
}
