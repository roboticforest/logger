import logger;

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "../test-support.hpp"

namespace {

using dvlogger_test::expect;
using dvlogger_test::has_log_prefix;
using dvlogger_test::message_payload;
using dvlogger_test::split_lines;

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

bool test_log_loop_invalid_range()
{
    std::ostringstream captured;
    DV::Logger log("LoopTest", captured);
    log_loop(log, 10, 10);

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == 2, "Invalid range should produce exactly two ERROR lines.");
    if (!ok) {
        return false;
    }

    ok &= expect(has_log_prefix(lines[0], "LoopTest", "ERROR"), "Invalid range first line prefix mismatch.");
    ok &= expect(has_log_prefix(lines[1], "LoopTest", "ERROR"), "Invalid range second line prefix mismatch.");
    ok &= expect(message_payload(lines[0]) == "logLoop() Test Failed!", "Invalid range first payload mismatch.");
    ok &= expect(message_payload(lines[1]) == R"(Variable "start" must be < or == variable "end".)",
                 "Invalid range second payload mismatch.");
    return ok;
}

bool test_log_loop_iteration_count()
{
    std::ostringstream captured;
    DV::Logger log("LoopTest", captured);
    log_loop(log, 0, 40);

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == 42, "Expected 42 lines (40 INFO + 2 DEBUG).");
    if (!ok) {
        return false;
    }

    int info_lines = 0;
    for (const std::string& line : lines) {
        if (has_log_prefix(line, "LoopTest", "INFO")) {
            ++info_lines;
        }
    }
    ok &= expect(info_lines == 40, "Expected 40 INFO lines from loop iterations.");
    return ok;
}

bool test_concurrent_line_integrity()
{
    std::ostringstream captured;
    DV::Logger log("ThreadTest", captured);

    constexpr int thread_count = 4;
    constexpr int iterations_per_thread = 120;

    auto worker = [&log](int worker_id) {
        for (int i = 0; i < iterations_per_thread; ++i) {
            log.info("Worker", worker_id, "iteration", i);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    for (int id = 0; id < thread_count; ++id) {
        threads.emplace_back(worker, id);
    }
    for (auto& t : threads) {
        t.join();
    }

    const std::vector<std::string> lines = split_lines(captured.str());
    bool ok = true;
    ok &= expect(lines.size() == thread_count * iterations_per_thread,
                 "Concurrent logging line count mismatch.");
    if (!ok) {
        return false;
    }

    for (const std::string& line : lines) {
        if (!has_log_prefix(line, "ThreadTest", "INFO")) {
            std::cerr << "Concurrent logging prefix mismatch: " << line << std::endl;
            return false;
        }
        const std::string payload = message_payload(line);
        if (payload.find("Worker ") != 0 || payload.find(" iteration ") == std::string::npos) {
            std::cerr << "Concurrent logging payload corruption detected: " << payload << std::endl;
            return false;
        }
    }
    return true;
}

}  // namespace

int main()
{
    bool ok = true;
    ok &= test_log_loop_invalid_range();
    ok &= test_log_loop_iteration_count();
    ok &= test_concurrent_line_integrity();
    return ok ? 0 : 1;
}
