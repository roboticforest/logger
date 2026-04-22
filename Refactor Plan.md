# Refactor Plan

## Phase 1: Lock the naming

- Pick one public identity and use it everywhere.
- Settle on:
  - package name: `DVLogger`
  - imported target: `DV::Logger`
  - internal build names: plain and explicit
- Remove mixed naming like `logger`, `dvlogger`, and `DVLogger` where it adds confusion.

## Phase 2: Flatten the top-level build

- Rewrite `CMakeLists.txt` around five plain sections:
  - project setup
  - libraries
  - install/export
  - tests
  - docs
- Delete helper layers that hide simple behavior.
- Keep only small helpers that remove obvious duplication.

## Phase 3: Simplify library targets

- Define the static and shared libraries directly.
- Keep shared/static naming explicit.
- Preserve only the essential shared-library Windows handling.
- Remove umbrella targets unless they provide real IDE value.

## Phase 4: Shrink install logic

- Keep standard CMake package export only.
- Make `find_package(DVLogger REQUIRED)` the primary install path.
- Ensure the exported package exposes `DV::Logger`.
- Keep the install layout simple enough for copied-source and submodule use without extra modes.

## Phase 5: Cut test plumbing

- Register tests in the most direct CMake form possible.
- Keep individual test executables visible to the IDE.
- Keep one clear aggregate target for normal tests.
- Leave manual/stress tests optional and plainly separated.
- Re-check whether the custom script and environment forwarding are still necessary; remove them if standard CTest is enough.

## Phase 6: Collapse docs config

- Make docs one optional feature with two outputs: public and internal.
- Remove repeated Doxygen setup.
- Do not build docs by default.
- Keep the docs section isolated from the core build.

## Phase 7: Rebuild the install smoke test

- Keep one installation smoke test.
- Make it verify the actual consumer story:
  - install package
  - `find_package(DVLogger REQUIRED)`
  - link `DV::Logger`
- Remove extra script complexity unless it protects a real Windows edge case.

## Phase 8: Clean and verify

- Delete dead targets, lists, variables, and comments.
- Update docs to match the simplified build.
- Verify:
  - both library variants build
  - unit tests run
  - optional manual tests still gate correctly
  - install smoke test passes
  - docs build when enabled

## End State

- One short, readable top-level build.
- One naming model.
- Standard install/export behavior.
- Simple test registration.
- Optional docs, not core build noise.
- Only the minimum convenience targets needed for IDE use.
