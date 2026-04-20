# CMake Simplification Plan

This plan compares the current config behavior to the goals in `Config Goals.md`, then proposes a reduction-first refactor.

## Goals check

### Goal 1: Build both library forms for Windows

Current state:

- Met in practice.
- The config builds one shared library and one static library.

Adjustment needed:

- Rename things so the intent is obvious.
- If "dynamic" in the goals really means "shared", use one term consistently everywhere.

### Goal 2: Extremely simple installation options

Current state:

- Partially met.
- Installed-package consumption works through a generated config package.
- The install smoke test verifies that path.

Gaps:

- The documented goal says `find_package(DVLogger REQUIRED)`, but the project currently tests `find_package(dvlogger CONFIG REQUIRED)`.
- The config does not clearly explain the "copy into project" or "git submodule add" workflows.
- The package and namespace naming are inconsistent with the stated public-facing name.

### Goal 3: Test thoroughly

Current state:

- Mostly met.
- There are tests for smoke, formatting, stream behavior, failure modes, threading, and installation.
- Manual/stress tests are optional.

Gaps:

- The config structure for registering tests is more complicated than the test matrix itself.
- Test naming is mostly clear, but the execution plumbing is not.

### Goal 4: All build and test targets visible to IDEs

Current state:

- Met, but with a lot of custom-target scaffolding.

Tradeoff:

- This goal is the main reason the file contains so many custom targets.
- Some of that scaffolding is justified, but some appears redundant.

## Simplification principles

The refactor should follow these rules:

1. Keep the build story obvious at first glance.
2. Prefer standard CMake behavior over custom wrappers.
3. Use helper functions only when they remove duplication without hiding intent.
4. Pick one public name for the package, namespace, and user-facing docs.
5. Separate optional concerns so build/install logic stays readable.
6. Remove targets and variables that exist only as indirection.

## Recommended target state

The final config should read like this:

1. Define the project and common settings.
2. Define the two library targets.
3. Define install/export rules.
4. Define tests.
5. Define docs only if requested.

That order matches how a novice thinks about a library project.

## Refactor plan

### Phase 1: Fix naming and reduce cognitive load

Use one public name consistently.

Recommended choice:

- Project/package name: `DVLogger`
- Export namespace: `DVLogger::`
- Installed package config path: `.../cmake/DVLogger`

Why:

- It matches the stated goal.
- It looks like a public library name rather than an internal variable prefix.
- It removes the current split between `logger`, `dvlogger`, and `DVLogger`.

Concrete changes:

- Change `project(logger ...)` to the chosen public name.
- Rename generated package files accordingly.
- Update the install smoke test consumer to use `find_package(DVLogger REQUIRED)`.
- Update aliases to match the public namespace.

Notes:

- Internal variable prefixes can still use `dvlogger_` if desired, but user-facing names should be unified.

### Phase 2: Flatten the library definition

Keep one helper only if it stays obviously readable.

Recommended approach:

- Keep a tiny helper for "create one library variant from the same sources".
- Move shared source lists and shared compile definitions into top-level variables with plain names.

Example shape:

- `set(dvlogger_sources logger.cpp)`
- `set(dvlogger_module_interface logger.cppm)`
- `function(add_dvlogger_library target type output_name)` or equivalent

Why:

- The current helper is reasonable, but `export_name` is not immediately obvious to a novice.
- Shared data should be named once and reused plainly.

Potential further simplification:

- If the helper still feels denser than the duplicated code, duplicate the few `target_*` lines instead.
- Two explicit `add_library(...)` blocks may be clearer than one clever helper.

This is a case where DRY must not outrank readability.

### Phase 3: Make install/export the "default story"

The install section is already relatively clean. Keep it standard.

Recommended changes:

- Keep `GNUInstallDirs` and `CMakePackageConfigHelpers`.
- Keep `configure_package_config_file(...)`.
- Keep `write_basic_package_version_file(...)`.
- Keep target export and module file-set install.

Reduce:

- Stop maintaining a manual `DVLOGGER_INSTALL_TARGETS` list if the installed targets are always known and fixed.
- Install the two library targets directly.

Why:

- There are only two installable library targets.
- A list variable adds indirection without saving real work.

### Phase 4: Simplify test registration aggressively

This is the biggest opportunity.

Recommended baseline:

- Keep normal `add_executable(...)`.
- Keep normal `add_test(NAME ... COMMAND $<TARGET_FILE:...>)`.
- Use `set_tests_properties(... LABELS ...)` for grouping.

Only keep the script-based launcher if it solves a real Windows problem that cannot be handled more simply.

Candidate simplifications:

- Replace `dvlogger_add_ctest_runner(...)` with a much smaller set of aggregate custom targets, or remove them entirely if the IDE already shows `test`.
- Replace `dvlogger_add_cpp_test(...)` with a helper that only does:
  - add executable
  - link library
  - add test
  - set labels
- Remove the unit/manual target lists if they are only used to feed custom runner targets.
- Remove `dvlogger_tests_normal` if it is just another name for `dvlogger_tests_unit`.

Important question to verify during implementation:

- Is `tests/run_built_test.cmake` still necessary with the current generator and IDE workflow?

If yes:

- Keep it, but narrow its role.
- Make the helper name reflect the actual reason, such as "run test through build step on Windows generators".

If no:

- Delete the script and the environment-forwarding machinery entirely.

Why this matters:

- The current custom runner layer is the largest single complexity source.
- If it can be removed, the file will shrink substantially and become much easier to read.

### Phase 5: Keep the install smoke test, but trim the plumbing

The install smoke test is valuable because it protects a key project goal.

Recommended changes:

- Keep `tests/install/install_smoke_test.cmake`.
- Keep the external consumer mini-project.
- Rename them consistently with the public package name.

Possible reductions:

- Replace the generic `dvlogger_run_or_fail(...)` helper with a smaller helper only if it still improves readability.
- Remove `dvlogger_rebuild_before_install` unless it is truly needed outside this one script.
- Revisit whether forwarding `INCLUDE`, `LIB`, and `LIBPATH` is still necessary once normal test launching is simplified.

The install smoke test should remain because it tests an important promise, but it should be the only custom script path unless others are clearly justified.

### Phase 6: Move docs out of the main path

The docs logic is the second-largest complexity source after testing.

Recommended changes:

- Keep docs optional.
- Do not build docs as part of `ALL` by default.
- Hide most of the Doxygen setup behind a small helper or, better, a separate included CMake file such as `cmake/docs.cmake`.

Why:

- Docs are not part of the core "can I build and use the library?" story.
- A novice reading the main `CMakeLists.txt` should not have to understand Doxygen configuration before understanding the library targets.

Reduction opportunities:

- Extract repeated public/internal Doxygen setup into one helper with parameters.
- Or split the entire docs section into a separate file and include it only when docs are enabled.

Preferred choice:

- Split docs into a separate file.

Reason:

- This removes a large block from the main file without adding conceptual trickiness.
- It is simpler than a dense general-purpose Doxygen helper.

### Phase 7: Re-evaluate umbrella targets

Current umbrella targets include:

- `dvlogger`
- `docs_all`
- `run_<individual test>`
- `dvlogger_tests_unit`
- `dvlogger_tests_normal`
- `dvlogger_tests_manual`
- `dvlogger_tests_all`

Recommended approach:

- Keep only the umbrella targets that clearly help IDE use.
- Remove duplicate or thin-wrapper targets.

Likely keep:

- one "build both libraries" target, if it is useful in the IDE
- one "run unit tests" target
- one "run all tests" target
- maybe one "run manual tests" target when manual tests are enabled

Likely remove:

- `dvlogger_tests_normal`
- `run_<individual test>` targets, unless there is a strong IDE workflow need for them
- `docs_all` if `docs_public` and `docs_internal` are already visible and clear

Why:

- Every extra target adds noise to IDE target lists.
- Goal 4 is about visibility, not about exposing every internal convenience wrapper.

## Suggested file structure after refactor

Recommended structure:

- Top-level `CMakeLists.txt`
  - project setup
  - library targets
  - install/export
  - `include(CTest)` and a short tests section
  - optional `include(cmake/docs.cmake)`
- `tests/install/install_smoke_test.cmake`
  - kept, but simplified if possible
- `tests/install/CMakeLists.txt`
  - updated to public naming
- `cmake/DVLoggerConfig.cmake.in`
  - renamed for public consistency

This keeps the top-level file focused on the actual build.

## Priority order

If the goal is maximum complexity reduction with minimum risk, refactor in this order:

1. Unify naming.
2. Remove redundant target lists and wrapper targets.
3. Simplify test registration.
4. Move docs out of the top-level file.
5. Re-check whether environment-forwarding scripts are still needed.

That order should produce useful readability gains early without breaking the core build/install behavior.

## Expected end result

A good end state would have:

- a much shorter top-level `CMakeLists.txt`
- one obvious naming scheme
- standard CMake packaging
- straightforward `add_test(...)` usage wherever possible
- optional docs isolated from the core build
- only a small number of IDE-facing convenience targets

Most importantly, a novice should be able to answer these questions by skimming the file once:

- What gets built?
- How do I link to it?
- How do I install it?
- How do I run tests?
- Which parts are optional?
