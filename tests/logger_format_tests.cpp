#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "logger.h"
#include "version.hpp"
#include "test_support.hpp"

namespace {

using dvlogger_test::expect;
using dvlogger_test::has_log_prefix;
using dvlogger_test::message_payload;
using dvlogger_test::split_lines;

bool test_levels_and_timestamp_shape()
{
    std::ostringstream captured;
    DV::Logger log("FormatTest", captured);

    log.debug("debug line");
    log.error("error line");
    log.fatal("fatal line");
    log.info("info line");
    log.trace("trace line");
    log.warn("warn line");

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == 6, "Expected 6 formatted log lines.");
    if (!ok) {
        return false;
    }

    ok &= expect(has_log_prefix(lines[0], "FormatTest", "DEBUG"), "DEBUG prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[1], "FormatTest", "ERROR"), "ERROR prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[2], "FormatTest", "FATAL"), "FATAL prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[3], "FormatTest", "INFO"), "INFO prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[4], "FormatTest", "TRACE"), "TRACE prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[5], "FormatTest", "WARN"), "WARN prefix/timestamp mismatch.");
    return ok;
}

bool test_message_assembly_spacing()
{
    std::ostringstream captured;
    DV::Logger log("FormatTest", captured);

    log.info("SingleMessage");
    log.info("Many", 5, 3.14, 'x');
    log.info("Version", LOGGER_VERSION_STRING);

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == 3, "Expected 3 lines for message assembly test.");
    if (!ok) {
        return false;
    }

    ok &= expect(message_payload(lines[0]) == "SingleMessage", "Single argument payload mismatch.");
    ok &= expect(message_payload(lines[1]) == "Many 5 3.14 x", "Variadic payload spacing mismatch.");
    ok &= expect(message_payload(lines[2]) == std::string("Version ") + LOGGER_VERSION_STRING,
                 "Version payload mismatch.");
    return ok;
}

}  // namespace

int main()
{
    bool ok = true;
    ok &= test_levels_and_timestamp_shape();
    ok &= test_message_assembly_spacing();
    return ok ? 0 : 1;
}
