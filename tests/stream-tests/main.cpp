import logger;

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../test-support.hpp"

namespace {

using dvlogger_test::expect;
using dvlogger_test::has_log_prefix;
using dvlogger_test::message_payload;
using dvlogger_test::split_lines;

bool test_split_streams_receive_identical_output()
{
    std::ostringstream primary;
    std::ostringstream split_a;
    std::ostringstream split_b;

    DV::Logger log("SplitTest", primary);
    log.addSplit(split_a);
    log.addSplit(split_b);

    log.info("Testing", "split", 123);

    const std::string out_primary = primary.str();
    const std::string out_a = split_a.str();
    const std::string out_b = split_b.str();

    bool ok = true;
    ok &= expect(!out_primary.empty(), "Primary output was empty.");
    ok &= expect(out_primary == out_a, "Primary and split A outputs were not identical.");
    ok &= expect(out_primary == out_b, "Primary and split B outputs were not identical.");

    const std::vector<std::string> lines = split_lines(out_primary);
    ok &= expect(lines.size() == 1, "Expected a single line in split stream test.");
    if (!ok) {
        return false;
    }

    ok &= expect(has_log_prefix(lines[0], "SplitTest", "INFO"), "Split stream line prefix mismatch.");
    ok &= expect(message_payload(lines[0]) == "Testing split 123", "Split stream payload mismatch.");
    return ok;
}

bool test_multiple_entries_fan_out_to_all_streams()
{
    std::ostringstream primary;
    std::ostringstream split_a;
    std::ostringstream split_b;
    std::ostringstream split_c;

    DV::Logger log("Multilog", primary);
    log.addSplit(split_a);
    log.addSplit(split_b);
    log.addSplit(split_c);

    log.debug("Logger created.");
    log.info("Testing output!");
    log.warn("Printing warnings.");

    bool ok = true;
    ok &= expect(primary.str() == split_a.str(), "Split A diverged from primary output.");
    ok &= expect(primary.str() == split_b.str(), "Split B diverged from primary output.");
    ok &= expect(primary.str() == split_c.str(), "Split C diverged from primary output.");

    const std::vector<std::string> lines = split_lines(primary.str());
    ok &= expect(lines.size() == 3, "Expected 3 fan-out log entries.");
    return ok;
}

}  // namespace

int main()
{
    bool ok = true;
    ok &= test_split_streams_receive_identical_output();
    ok &= test_multiple_entries_fan_out_to_all_streams();
    return ok ? 0 : 1;
}
