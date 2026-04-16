#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "logger.h"
#include "test_support.hpp"

namespace {

using dvlogger_test::expect;

bool test_closed_file_stream_behavior()
{
    const std::filesystem::path file_path = "bad-stream-test.log";
    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open test file stream." << std::endl;
        return false;
    }

    DV::Logger log("BadStream", file);
    log.info("Closing file...");

    file.close();

    bool threw = false;
    try {
        log.error("This message will be in error.");
    } catch (...) {
        threw = true;
    }

    bool ok = true;
    ok &= expect(!threw, "Logger threw while writing to a closed file stream.");
    ok &= expect(!file.is_open(), "File stream was unexpectedly open.");
    ok &= expect(file.fail() || file.bad(), "Closed stream write did not set fail/bad state as expected.");

    std::error_code ec;
    std::filesystem::remove(file_path, ec);
    return ok;
}

}  // namespace

int main()
{
    return test_closed_file_stream_behavior() ? 0 : 1;
}
