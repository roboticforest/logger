# Upgrade Milestone 2 (Temporary Working Plan)

Goal: modernize logger internals toward C++20 modules with no behavior change.

## Current milestone plan (high-level)

1. Freeze existing behavior with dedicated tests.
2. Add a stable non-template logging backend entrypoint.
3. Keep public variadic API, but make it thin and forwarding-only.
4. Move mutable state and implementation internals behind pImpl.
5. Add C++20 module interface and implementation units.
6. Keep header-based compatibility path during transition.
7. Update CMake to support both include-based and module-based consumption.
8. Validate parity and remove obsolete transitional pieces.

---

## Focus plan: Step 1 only (behavior freeze with tests)

### Objective

Capture current logger behavior in tests before structural refactors, so later module/pImpl changes can be verified against a stable baseline.

### Scope

- Output content and format (header + message assembly).
- Log level labeling.
- Multi-stream split behavior.
- Existing threading expectations from current stress scenarios.
- Version macros/basic integration expectations.

### Deliverables

1. A new test suite that maps each major existing stress block into formal tests.
2. Optional separation of fast tests vs stress tests (via labels).
3. Deletion of root `main.cpp` after test parity is established.
4. CI/local command guidance for running all tests and stress-only tests.

### Execution strategy

1. Introduce test organization under `tests/`:
- `logger_format_tests.cpp`
- `logger_stream_tests.cpp`
- `logger_thread_tests.cpp`
- `logger_failure_mode_tests.cpp` (for known bad-stream scenarios that should not crash test runner)
- Keep existing `dvlogger_smoke_test.cpp` temporarily.

2. Convert each legacy stress block from `main.cpp` into targeted tests.

3. Preserve behavior, not style:
- Do not “fix” behavior while writing baseline tests.
- For fragile areas, assert only stable invariants.

4. Add test labels for runtime profile:
- `unit` for fast deterministic checks.
- `stress` for heavy loops / thread contention checks.
- `manual` for tests that intentionally exercise dangerous patterns (if retained).

5. Remove `main.cpp` only after:
- all mapped scenarios exist as tests,
- builds pass in Debug,
- CTest registration is complete.

---

## Deconstructing `main.cpp` into formal tests

This is the direct migration map for the existing test harness code blocks.

### Block-by-block mapping

1. `basicTest(DV::Logger&)`
- New tests:
  - `logs_all_levels_with_expected_level_tokens`
  - `accepts_mixed_variadic_argument_types`
  - `handles_single_argument_and_multi_argument_entries`
- Assertions:
  - output contains logger name + each level token (`INFO`, `WARN`, etc.)
  - message tokens are present in expected order
  - no missing spaces between assembled message parts

2. `logLoop(DV::Logger&, int, int)`
- New tests:
  - `logs_expected_iteration_count_for_range`
  - `rejects_invalid_range_and_reports_error`
- Notes:
  - Keep a moderate loop size for default test runtime.
  - Add a stress variant with larger ranges under `stress` label.

3. `threadTest(DV::Logger&)`
- New tests:
  - `concurrent_logging_produces_nonempty_ordered_lines`
  - `concurrent_logging_preserves_line_integrity`
- Assertions:
  - expected rough line count exists
  - each produced line starts with timestamp/header prefix pattern
  - no obvious interleaving fragments (partial bracket/header corruption)

4. `badStreamTest(DV::Logger&)`
- New tests:
  - `closed_file_stream_sets_stream_bad_state_after_write_attempt`
  - `logger_continues_without_throwing_on_bad_stream`
- Important:
  - assert stream state transitions and non-crash behavior
  - avoid assumptions about recovering closed file stream output

5. `deadStreamTest(DV::Logger&)`
- Recommendation:
  - do not run as a normal automated test (intentionally invokes UB / possible crash).
- Options:
  - keep as disabled/manual-only documentation case, or
  - remove from automated suite and document as unsupported unsafe use-case.

6. `teeStreamTest()`
- New tests:
  - `addSplit_duplicates_output_to_multiple_streams`
  - `split_streams_receive_same_message_payload`
- Assertions:
  - all captured outputs include same entry payload and headers.
  - color behavior expectations remain minimal unless explicitly stable.

7. Root `main()` custom scenario (`multilog` + 3 file splits + `threadTest`)
- New test:
  - `multistream_concurrent_stress_smoke`
- Label:
  - `stress`

### Proposed removal path for `main.cpp`

1. Port all blocks above.
2. Confirm CTest runs green with equivalent coverage.
3. Remove `main.cpp` from build inputs (if currently built) and delete file.
4. Keep historical intent in test names and comments so original stress harness purpose is preserved.

---

## Open questions before implementing Step 1

1. Should dangerous scenarios (like deleted stream pointer usage) be kept as manual docs/tests or fully removed?
2. Do you want strict format assertions for timestamp shape (`[TZ YYYY-MM-DD HH:MM:SS:NNNNNNNNN]`) or only token-level checks?
3. Should heavy-count/thread stress tests run by default in local `ctest`, or be opt-in via label?
