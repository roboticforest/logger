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

## Full execution plan (module-only migration with `.cppm`)

### Objective

Ship `logger` as a C++20 module-first library where consumers use:

```cpp
import logger;
```

with no public include-based API path.

### Guiding decisions

- `.cppm` is the canonical extension for module interface units.
- Public surface is exported from `logger.cppm` only.
- Legacy public headers (`logger.h`, generated version macro header) are removed from install/consumer path.
- Version info becomes exported constants/functions, not macros.
- pImpl and behavior-parity constraints remain in place unless explicitly changed.

### Scope

- In scope:
  - Module interface + module implementation units.
  - CMake module file sets and install/export for module consumption.
  - Tests updated to `import logger;`.
  - Docs and examples updated to module usage.
- Out of scope:
  - Backward compatibility for non-module toolchains.
  - Dual include/module shipping model.

### Target file layout

- `logger.cppm`: primary module interface unit (`export module logger;`).
- `logger.cpp`: module implementation unit (`module logger;`) containing private implementation details.
- `cmake/version.hpp.in`: retired from public API usage; replace with compile definitions or configured constants injected into module build.
- `logger.h`: transitional internal artifact only during migration, then removed from install and eventually deleted.

### Public API design constraints

1. Preserve existing `DV::Logger` behavior and function names:
- `debug`, `error`, `fatal`, `info`, `trace`, `warn`, `addSplit`.

2. Keep entrypoint signatures usable exactly as today:
- variadic logging entrypoints remain templates or become exported forwarding wrappers with equivalent call behavior.

3. Preserve output contract:
- header format, spacing, level tags, split-stream behavior, and thread-safety semantics remain unchanged.

4. Replace version macros with module exports:
- example target:
  - `export class DV::Logger { struct Version { static inline constexpr int major = ...; ... }; ... };`
  - `DV::Logger::Version::string` exposes semantic version text.

### Work breakdown

1. Pre-migration freeze
- Lock existing behavior with current automated test suite.
- Add any missing assertions for output details likely to regress in module split.

2. Introduce module interface (`logger.cppm`)
- Create `export module logger;`.
- Move public includes/imports needed for exported declarations.
- Export `namespace DV` and `class Logger` API.
- Export version constants replacing macro usage.

3. Convert implementation unit
- Update `logger.cpp` to `module logger;` implementation context.
- Keep private helpers (`TerminalColor`, formatting logic, `Logger::Impl`) non-exported.
- Ensure lock scope and emit path remain exactly one critical section.

4. Retire header-first API path
- Remove `logger.h` from target public headers and installs.
- Remove generated public `version.hpp` from installs.
- Keep temporary local compatibility shim only if needed to complete transition, then delete.

5. CMake module wiring
- Define module source file set for each library target (`dvlogger_shared`, `dvlogger_static`).
- Ensure module interface unit is in a `CXX_MODULES` file set and installed/exported.
- Keep `cxx_std_20` or raise only if required by compiler behavior.
- Validate `find_package(dvlogger)` consumer can compile with `import logger;`.

6. Consumer/test migration
- Update tests from `#include "logger.h"`/`#include "version.hpp"` to `import logger;`.
- Remove macro-based version checks and assert exported version constants.
- Update docs/examples/package snippets accordingly.

7. Parity and cleanup
- Run full test matrix (including optional manual tests when enabled).
- Remove dead transitional code/comments/files.
- Confirm no installed public headers are required for normal consumption.

### Verification matrix

- Unit tests:
  - `dvlogger.smoke`
  - `dvlogger.format`
  - `dvlogger.stream`
  - `dvlogger.thread`
  - `dvlogger.failure_mode`
- Optional:
  - `dvlogger.manual.stress`
  - `dvlogger.manual.dead_stream`
- Packaging checks:
  - Build + install static and shared variants.
  - Consume from a clean external sample target via `find_package(dvlogger CONFIG REQUIRED)` and `import logger;`.
  - Validate Debug/Release both work (module artifacts are configuration-specific).

### Risks and mitigations

- Risk: compiler/CMake module integration edge cases across generators.
  - Mitigation: keep Ninja + MSVC as reference path; verify install/export with clean consumer build early.

- Risk: macro removal breaks downstream version checks.
  - Mitigation: provide clearly named exported constants and update docs/examples in same change window.

- Risk: accidental behavior drift during module split.
  - Mitigation: preserve backend seam and lock placement; enforce parity with existing tests before cleanup.

### Definition of done

1. External consumers can use only `import logger;` with installed package targets.
2. No public include of `logger.h` is required or documented.
3. Version information is available through exported module API (no required macros).
4. Existing behavior and test suite remain green with no intentional format changes.
5. Transitional include-era files are removed from installation and no longer part of supported usage.
