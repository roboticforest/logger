# Upgrade Milestone 2 (Temporary Working Plan)

Goal: modernize logger internals toward C++20 modules with no behavior change.

## Current milestone plan (high-level)

1. ~~Freeze existing behavior with dedicated tests.~~
2. ~~Add a stable non-template logging backend entrypoint.~~
3. ~~Keep public variadic API, but make it thin and forwarding-only.~~
4. ~~Move mutable state and implementation internals behind pImpl.~~
5. Add C++20 module interface and implementation units.
6. Keep header-based compatibility path during transition.
7. Update CMake to support both include-based and module-based consumption.
8. Validate parity and remove obsolete transitional pieces.

---

## Focus plan: Step 2 only (port logging backend seam)

### Objective

Introduce a stable non-template backend path for log emission so variadic templates become thin front-end adapters and future module/pImpl work can proceed without behavior drift.

### Scope

- Preserve public API shape (`debug/error/fatal/info/trace/warn`) and output format.
- Keep all existing lock timing and write sequencing behavior stable for this step.
- Add one non-template backend call path in `logger.cpp` used by all public variadic entry points.
- Avoid pImpl/module file moves in this step; this is seam creation only.

### Deliverables

1. `Logger` gets one internal non-template backend function for final emit.
2. Public variadic methods become thin wrappers that:
- assemble message text,
- then call backend with `LogLevel` + assembled payload.
3. Existing tests remain green with no output changes.
4. Add a short code note explaining that the seam exists to enable module/pImpl migration in later steps.

### Execution strategy

1. Define the seam in class internals.
- Add a private non-template method signature (target shape): `emit(LogLevel, std::string_view payload)`.
- Keep `buildHeader()` and `write()` behavior unchanged initially.

2. Split message assembly from emission.
- Keep variadic packing in header-visible code.
- Change assembly helpers to produce payload text only (no header/write side effects).
- Route all level methods through the shared backend.

3. Centralize synchronization in backend.
- Move lock ownership to the new backend function with RAII (`std::lock_guard<std::mutex>`).
- Ensure header + payload + write remain one critical section exactly as today.

4. Limit copy churn without changing behavior.
- Prefer forwarding references in template wrappers where safe.
- Preserve exact spacing and payload construction order.

5. Keep this step behavior-neutral.
- Do not fix timestamp formatting yet.
- Do not alter split-stream/color behavior.
- Do not change ownership/lifetime model yet.

6. Verify with the current test matrix.
- `dvlogger.smoke`
- `dvlogger.format`
- `dvlogger.stream`
- `dvlogger.thread`
- `dvlogger.failure_mode`
- Optional manual tests with `DVLOGGER_ENABLE_MANUAL_TESTS=ON`.
