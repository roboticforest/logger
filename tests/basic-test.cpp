import logger;

#include <array>
#include <regex>
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
    log.info("Single string literal argument.");
    log.trace("Single string literal argument.");
    log.warn("Single string literal argument.");

    log.debug("Many", "string literals", "passed in", "all", "together.");
    log.error("Many", "string literals", "passed in", "all", "together.");
    log.fatal("Many", "string literals", "passed in", "all", "together.");
    log.info("Many", "string literals", "passed in", "all", "together.");
    log.trace("Many", "string literals", "passed in", "all", "together.");
    log.warn("Many", "string literals", "passed in", "all", "together.");

    std::string message = "Various types: ";
    log.debug(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.error(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.fatal(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.info(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.trace(message, 5, 3.14f, 'a', "b c", &message, true, false);
    log.warn(message, 5, 3.14f, 'a', "b c", &message, true, false);

    const OutputForwarder output_forwarder(argc, argv, captured);
    output_forwarder.forward_output();

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == (LEVEL_NAMES.size() * 3), "Expected 18 formatted log lines.");
    if(!ok) { return 1; }

    for(std::size_t line_idx = 0; line_idx < lines.size(); ++line_idx) {
        const std::regex mixed_types_message_pattern(
            R"(^Various types:  5 3\.14 a b c (0x)?[0-9A-Fa-f]+ 1 0$)"
        );
        constexpr std::array<std::string_view, 2> expected_messages = {
            "Single string literal argument.",
            "Many string literals passed in all together."
        };
        const std::string& line = lines[line_idx];
        const std::string_view level_name = LEVEL_NAMES[line_idx % LEVEL_NAMES.size()];
        const std::size_t message_idx = line_idx / LEVEL_NAMES.size();
        const std::string level_label(level_name);
        const std::string payload = message_payload(line);

        ok &= expect(
            has_log_prefix(line, "BasicTest", level_name),
            "Prefix/timestamp mismatch for line " + std::to_string(line_idx) + " (" + level_label + ")."
        );

        if(message_idx < expected_messages.size()) {
            ok &= expect(
                payload == expected_messages[message_idx],
                "Payload mismatch for line " + std::to_string(line_idx) + " (" + level_label + ")."
            );
            continue;
        }

        ok &= expect(
            std::regex_match(payload, mixed_types_message_pattern),
            "Mixed-type payload format mismatch for line " + std::to_string(line_idx) + " (" + level_label + ")."
        );
    }

    return ok ? 0 : 1;
}
