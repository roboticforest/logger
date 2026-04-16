# Logger Upgrade Notes (Milestone 2)

This file records known issues and risks identified before the C++20 modules modernization work.

## Key findings from current code review

1. Exception-unsafe mutex handling in variadic logging path.
- Location: `logger.h` (`Logger::assemble(LogLevel, ...)`).
- Detail: Uses manual `_writeMutex.lock()` / `_writeMutex.unlock()`.
- Risk: If stream insertion throws, mutex can remain locked and deadlock future logging calls.

2. Data race when adding split streams during logging.
- Location: `logger.cpp` (`addSplit`) and `logger.cpp` (`write`).
- Detail: `_streams` is mutated in `addSplit()` without locking while `write()` iterates the same vector.
- Risk: Concurrent mutation/iteration is undefined behavior.

3. Thread-safety concern in timestamp formatting.
- Location: `logger.cpp` (`buildHeader`).
- Detail: Uses `std::localtime`, which relies on shared static state.
- Risk: Concurrent calls from multiple logger instances can race in time formatting.

4. Missing direct include for used type in public header.
- Location: `logger.h`.
- Detail: Uses `std::reference_wrapper` but does not include `<functional>` directly.
- Risk: Transitive include dependence can break with toolchain/header changes.

5. Logger name lifetime risk.
- Location: `logger.h` / `logger.cpp` (`_name` and constructor).
- Detail: `_name` is stored as `const char*`.
- Risk: Caller can pass a pointer with shorter lifetime than the logger instance.

6. Unnecessary argument copies in variadic API.
- Location: `logger.h` (`debug/error/fatal/info/trace/warn` and assembly helpers).
- Detail: Parameters are passed by value through template layers.
- Risk: avoidable copies, especially for expensive string/object inputs.

## Constraints for milestone 2

- Preserve externally visible behavior and output format.
- Do not change level names, spacing conventions, timestamp layout, or stream routing semantics in this milestone.
- Any internal safety fixes must be evaluated against behavior parity tests first.
