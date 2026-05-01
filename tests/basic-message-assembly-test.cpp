import logger;

#include <sstream>
#include <string>
#include <vector>

#include "test_support.hpp"

int main() {
    using dvlogger_test::expect;
    using dvlogger_test::message_payload;
    using dvlogger_test::split_lines;

    std::ostringstream captured;
    DV::Logger log("FormatTest", captured);

    log.info("SingleMessage");
    log.info("Many", 5, 3.14, 'x');
    log.info("Version", DV::Logger::Version::string);

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == 3, "Expected 3 lines for message assembly test.");
    if(!ok) { return 1; }

    ok &= expect(message_payload(lines[0]) == "SingleMessage", "Single argument payload mismatch.");
    ok &= expect(message_payload(lines[1]) == "Many 5 3.14 x", "Variadic payload spacing mismatch.");
    ok &= expect(message_payload(lines[2]) == std::string("Version ") + std::string(DV::Logger::Version::string),
                 "Version payload mismatch.");
    return ok ? 0 : 1;
}
