import logger;

#include <array>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "test-support.hpp"

constexpr std::array<std::string_view, 6> LEVEL_NAMES = {
    "DEBUG",
    "ERROR",
    "FATAL",
    "INFO",
    "TRACE",
    "WARN"
};

int main(const int argc, char* argv[]) {
    using dvlogger_test::expect;
    using dvlogger_test::has_log_prefix;
    using dvlogger_test::message_payload;
    using dvlogger_test::OutputForwarder;
    using dvlogger_test::split_lines;

    std::ostringstream captured;
    DV::Logger log("BasicTest", captured);

    log.debug("Single string literal argument.");
    log.error("Single string literal argument.");
    log.fatal("Single string literal argument.");
    log.info ("Single string literal argument.");
    log.trace("Single string literal argument.");
    log.warn ("Single string literal argument.");

    log.debug("Many", "string literals", "passed in", "all", "together.");
    log.error("Many", "string literals", "passed in", "all", "together.");
    log.fatal("Many", "string literals", "passed in", "all", "together.");
    log.info ("Many", "string literals", "passed in", "all", "together.");
    log.trace("Many", "string literals", "passed in", "all", "together.");
    log.warn ("Many", "string literals", "passed in", "all", "together.");

    std::string message = "Various types: ";
    log.debug(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.error(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.fatal(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.info (message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.trace(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.warn (message, 5, 3.14f, 'a', "b c", &message, true, false);

    const OutputForwarder output_forwarder(argc, argv, captured);
    output_forwarder.forward_output();

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == (LEVEL_NAMES.size() * 3), "Expected 18 formatted log lines.");
    if(!ok) { return 1; }
    return 0;

    // Unfinished and broken block to verify the content of each message.
    // It should check that the overall format is correct, and that the
    // expected messages are correct.

    // Verifying that a memory address was output correctly, or that the
    // timestamps in the header are correct, should be done with some
    // flexibility and not simple string comparisons.

    // TODO: Finish this verification block. On hold until timestamps are fixed.

    //
    //
    // std::size_t line_idx = 0;
    // const auto expect_group = [&](const std::string& expected_payload, const std::string& group_name) {
    //     for(const std::string_view level_name : LEVEL_NAMES) {
    //         const std::string& line = lines[line_idx];
    //         const std::string level_label(level_name);
    //         ok &= expect(
    //             has_log_prefix(line, "BasicTest", level_name),
    //             group_name + " prefix/timestamp mismatch for " + level_label + "."
    //         );
    //         ok &= expect(
    //             message_payload(line) == expected_payload,
    //             group_name + " payload mismatch for " + level_label + "."
    //         );
    //         ++line_idx;
    //     }
    // };
    //
    // expect_group(single_message, "Single-message");
    // expect_group(many_messages, "Many-message");
    // expect_group(mixed_types_message, "Mixed-types");
    //
    // return ok ? 0 : 1;
}
