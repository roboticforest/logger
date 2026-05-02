import logger;

#include <sstream>
#include <string>
#include <vector>

#include "test-support.hpp"

int main(const int argc, char* argv[])
{
    using dvlogger_test::expect;
    using dvlogger_test::has_log_prefix;
    using dvlogger_test::OutputForwarder;
    using dvlogger_test::split_lines;

    std::ostringstream captured;
    DV::Logger log("FormatTest", captured);

    log.debug("debug line");
    log.error("error line");
    log.fatal("fatal line");
    log.info("info line");
    log.trace("trace line");
    log.warn("warn line");

    const OutputForwarder output_forwarder(argc, argv, captured);
    output_forwarder.forward_output();

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == 6, "Expected 6 formatted log lines.");
    if (!ok) {
        return 1;
    }

    ok &= expect(has_log_prefix(lines[0], "FormatTest", "DEBUG"), "DEBUG prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[1], "FormatTest", "ERROR"), "ERROR prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[2], "FormatTest", "FATAL"), "FATAL prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[3], "FormatTest", "INFO"), "INFO prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[4], "FormatTest", "TRACE"), "TRACE prefix/timestamp mismatch.");
    ok &= expect(has_log_prefix(lines[5], "FormatTest", "WARN"), "WARN prefix/timestamp mismatch.");
    return ok ? 0 : 1;
}
