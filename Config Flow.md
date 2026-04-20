# CMake Config Flow Notes

This file documents what the current CMake config appears to do, and my best guess as to why each part exists.

## High-level shape

The current build is centered in the top-level `CMakeLists.txt`. It handles four concerns in one file:

1. Project setup.
2. Library target creation.
3. Documentation generation.
4. Test registration and test runner targets.
5. Install/package export.

This means the file is functionally complete, but it also means the "main story" of the build is buried inside a lot of supporting machinery.

## Project setup

The file starts with:

- `cmake_minimum_required(VERSION 4.0)`
- `project(logger VERSION 0.90.0.0 LANGUAGES CXX)`
- `include(GNUInstallDirs)`
- `include(CMakePackageConfigHelpers)`
- `find_package(Threads REQUIRED)`

What it is doing:

- Declares a C++ project called `logger`.
- Imports install directory helpers such as `CMAKE_INSTALL_LIBDIR`.
- Imports package-config helpers for generating `*-config.cmake` files.
- Requires the thread library up front.

Why it probably exists:

- The library uses threads directly, so `Threads::Threads` is a normal public dependency.
- Packaging helpers are needed because the project is exporting CMake package files for `find_package(...)`.

## Build options

The file defines four options:

- `DVLOGGER_BUILD_PUBLIC_DOCS`
- `DVLOGGER_BUILD_INTERNAL_DOCS`
- `DVLOGGER_ENABLE_MANUAL_TESTS`
- `DVLOGGER_ENABLE_INSTALL`

What it is doing:

- Lets docs, manual tests, and install rules be turned on or off.

Why it probably exists:

- Docs and manual tests are optional developer conveniences.
- Install rules are optional so the project can also be embedded without exporting package files.

## Library target creation

The config defines a helper function:

- `dvlogger_add_target(target_name library_type output_name export_name)`

What that function does:

- Creates a library target.
- Requires C++20.
- Adds `logger.cpp` as a normal source file.
- Adds `logger.cppm` as a `CXX_MODULES` file set.
- Publishes version information through compile definitions.
- Links `Threads::Threads`.
- Sets `OUTPUT_NAME` and `EXPORT_NAME`.

The function is then used twice:

- `dvlogger_shared` as `SHARED`, output name `dvlogger`, export name `shared`
- `dvlogger_static` as `STATIC`, output name `dvlogger_static`, export name `static`

Other related targets:

- `WINDOWS_EXPORT_ALL_SYMBOLS` is enabled for the shared library.
- Alias targets are created:
  - `dvlogger::shared`
  - `dvlogger::static`
- A custom target named `dvlogger` depends on both libraries.

What this means in practice:

- The project builds both static and shared variants.
- Consumers inside the same build can link to the aliases.
- The `dvlogger` custom target acts as an IDE-visible umbrella target.

Why it probably exists:

- The helper function avoids duplicating the shared/static setup.
- `WINDOWS_EXPORT_ALL_SYMBOLS` avoids writing export macros by hand.
- The umbrella target exists so an IDE can show a single obvious "build both" target.

Notes:

- The goal file says "shared and dynamic library for Windows." My best guess is that this means "static and shared", because those are the actual two outputs being built.
- `export_name` is only used so the installed export produces `dvlogger::shared` and `dvlogger::static`.
- The project name is `logger`, but the exported package/targets are named `dvlogger`. That naming split increases mental load.

## Documentation section

Public docs are controlled by `DVLOGGER_BUILD_PUBLIC_DOCS`. Internal docs are nested under that.

What the public docs logic does:

- Finds Doxygen.
- Defines a long list of doc-related variables.
- Builds a footer file from `cmake/DoxygenFooter.html.in`.
- Builds a Doxygen config from `cmake/Doxyfile.public.in`.
- Creates custom target `docs_public`.
- Adds `docs_public` to `ALL`, so it builds by default.

What the internal docs logic does:

- Reuses much of the public-doc state.
- Adds `logger.cpp` to the input set.
- Builds a second Doxygen config from `cmake/Doxyfile.internal.in`.
- Creates custom target `docs_internal`.

Then it creates:

- `docs_all`, which depends on `docs_public`
- `docs_all`, optionally also depends on `docs_internal`

Why it probably exists:

- The project wants separate public and internal documentation outputs.
- Internal docs turn on more expensive Doxygen features like call graphs.
- `docs_all` is another IDE-friendly umbrella target.

Notes:

- This section contains a lot of repeated variable setup and repeated `configure_file(...)` / `add_custom_target(...)` structure.
- `docs_public` being part of `ALL` means a normal build also tries to build documentation. That is convenient for maintainers, but it is unusual for a library and may surprise consumers or novice contributors.
- There are variables defined for copyright and license text that appear to exist for template substitution.

## Testing section

Testing starts with `include(CTest)` and then checks `if(BUILD_TESTING)`.

### Test runner helper

The config defines `dvlogger_add_ctest_runner(...)`.

What it does:

- Builds a `ctest --output-on-failure` command.
- Optionally adds a label filter.
- Optionally adds a regex filter.
- Wraps that command in a custom target.

Why it probably exists:

- CTest tests are not always as obvious in IDEs as normal build targets.
- This helper creates IDE-visible "run tests" targets.

### Environment escaping helper

The config defines `dvlogger_escape_semicolons(...)`.

What it does:

- Escapes semicolons in environment variables before passing them through `-D...` script arguments.

Why it probably exists:

- On Windows, `INCLUDE`, `LIB`, and `LIBPATH` contain semicolon-separated paths.
- In CMake, semicolons are list separators, so passing them through command arguments requires care.

### C++ test helper

The config defines `dvlogger_add_cpp_test(target_name source_file test_group test_labels)`.

What it does:

- Creates a test executable.
- Requires C++20.
- Links the static library.
- Registers a CTest test.
- The CTest test does not run the executable directly.
- Instead, it launches `cmake -P tests/run_built_test.cmake`.
- That helper script rebuilds the named test target, then runs the produced executable.
- Labels are attached to the test.
- The executable name is added to either a unit-test list or a manual-test list.
- A matching `run_<target>` custom target is created for IDE visibility.

Why it probably exists:

- Running the script instead of the test executable ensures the test target is built before execution, even when CTest is launched directly.
- Capturing the environment variables appears intended to preserve MSVC developer-shell paths when CTest is not launched from a fully configured shell.
- The list bookkeeping supports aggregate targets later.

Notes:

- This is the most complex part of the file.
- It solves real Windows/IDE problems, but it does so with a custom layer on top of normal CTest behavior.
- The complexity cost is high compared to the amount of actual test registration.

### Registered tests

Unit tests registered:

- `dvlogger_smoke_test`
- `dvlogger_format_test`
- `dvlogger_stream_test`
- `dvlogger_thread_test`
- `dvlogger_failure_mode_test`

Optional manual tests:

- `dvlogger_stress_manual_test`
- `dvlogger_dead_stream_manual_test`

Aggregate runner targets:

- `dvlogger_tests_unit`
- `dvlogger_tests_normal` as an alias-like custom target depending on `dvlogger_tests_unit`
- `dvlogger_tests_manual`
- `dvlogger_tests_all`

Why they probably exist:

- The named groups reflect the testing goals in `Config Goals.md`.
- The aggregate targets give obvious run points in an IDE.

### Install smoke test

If install rules are enabled, the config adds `dvlogger_installation_smoke_test`.

What it does:

- Registers a CTest test that runs `tests/install/install_smoke_test.cmake`.
- That script:
  - deletes scratch install/build folders
  - optionally rebuilds both library variants
  - installs the project to an isolated prefix
  - configures a tiny external consumer project in `tests/install`
  - builds that consumer
  - runs the resulting executable

The consumer project:

- calls `find_package(dvlogger CONFIG REQUIRED)`
- links `dvlogger::static`

Why it probably exists:

- It verifies that installation/export metadata is actually usable by another project.
- It also verifies that C++ modules packaging works well enough for a consumer build.

Notes:

- This is directly aligned with the installation goal, but the package name used is `dvlogger`, not `DVLogger`.
- The helper script has some reasonable reuse via `dvlogger_run_or_fail(...)`.
- The script forcibly removes the scratch directories each run to avoid stale state.

## Install and package section

When `DVLOGGER_ENABLE_INSTALL` is on, the config:

- chooses `${CMAKE_INSTALL_LIBDIR}/cmake/dvlogger` as the package config install directory
- generates:
  - `dvlogger-config.cmake`
  - `dvlogger-config-version.cmake`
- installs both library targets
- exports them as `dvloggerTargets`
- installs the module file set
- installs the export file with namespace `dvlogger::`
- installs the generated config files

The package config template:

- includes `CMakeFindDependencyMacro`
- calls `find_dependency(Threads)`
- includes `dvloggerTargets.cmake`

Why it probably exists:

- This is the standard CMake package-export path for `find_package(...)`.
- The `Threads` dependency is forwarded so consumers do not need to find it manually.

Notes:

- This is the cleanest part of the file. It uses standard CMake packaging helpers.
- The main readability issue here is naming consistency, not the install logic itself.

## Current behavior versus likely intent

The config appears to be trying to satisfy four goals at once:

1. Build both library variants.
2. Expose convenient install/package behavior.
3. Make tests and docs easy to run from an IDE.
4. Preserve enough Windows-specific handling that tests and installs work outside a perfect developer shell.

That intent is reasonable. The problem is that the file now optimizes heavily for "helper machinery" and less for "a novice can read this and understand the build in one pass."

## Main sources of complexity

The biggest complexity drivers are:

- Multiple concerns mixed together in one top-level file.
- Repeated Doxygen setup logic.
- A custom CTest runner layer built around scripts and custom targets.
- Environment-variable preservation plumbing for tests.
- Inconsistent naming between `logger`, `dvlogger`, and the goals document's `DVLogger`.
- Several umbrella targets whose purpose is convenience, but whose presence makes the target graph noisier.

## Best-guess summary

My best guess is that the config started from straightforward build/install needs, then accumulated extra helpers to fix real usability problems:

- IDE visibility of build/test/doc actions
- Windows shared-library exporting
- CTest invocation reliability
- install-package verification

Those fixes were added locally and successfully, but not consolidated afterward. The result is a config that works like a toolkit rather than a simple build description.
