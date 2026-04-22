# CMake Simplification Plan

This plan compares the current config behavior to the goals in `Config Goals.md`, then proposes a reduction-first refactor.

It also incorporates the follow-up clarifications:

- The consumer-facing identity should avoid collisions and should likely center on `DV::Logger`.
- A novice-friendly top-level overview is desirable, even if implementation details are split into included CMake files.
- Docs only need two modes: public and internal, with an "all or nothing" build switch.
- Testing should optimize for "run everything" by default, while still making individual tests available when useful.

## Goals check

### Goal 1: Build both library forms for Windows

Current state:

- Met in practice.
- The config builds one shared library and one static library.

Adjustment needed:

- Rename things so the intent is obvious.
- Use `static` and `shared` consistently in the config, comments, and target names.

### Goal 2: Extremely simple installation options

Current state:

- Partially met.
- Installed-package consumption works through a generated config package.
- The installation smoke test verifies that path.

Gaps:

- The project/package name, exported targets, and C++ namespace story are not aligned clearly.
- The config does not clearly explain the "copy into project" or "git submodule add" workflows.
- The package naming should reduce collision risk for consumers, even though the repository/project itself is just `logger`.

### Goal 3: Test thoroughly

Current state:

- Mostly met.
- There is a simple smoke test, as well as one for formatting, stream behavior, failure modes, threading, and installation.
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

## Naming direction

Required naming model:

- Project and Consumer-facing `find_package()` name: `DVLogger`
- C++ and CMake namespace: `DV`
- C++ module and library identity: `Logger`
- Consumer-facing imported target: `DV::Logger`

Why:

- While `logger` remains the simple conversational name for the project, the above code naming rules provide consistency and prevent simple typing errors.
- `DV::...` remains the collision-resistant public namespace family across several libraries.
- Consumers are protected from overly generic target names like `logger::shared`, which could refer to any library.

## Simplification principles

The refactor should follow these rules:

1. Keep the build story obvious at first glance.
2. Prefer standard CMake behavior over custom wrappers.
3. Use helper functions only when they remove duplication without hiding intent.
4. Keep the top-level file readable as an overview, even if details are split into included files.
5. Separate optional concerns so build/install logic stays readable.
6. Remove targets and variables that exist only as indirection.
7. Treat public consumer naming separately from internal variable naming.

## Recommended target state

The final config should read like this:

1. Define the project and common settings.
2. Define the two library targets.
3. Define install/export rules.
4. Define tests.
5. Define docs only if requested.

That order matches how a novice thinks about a library project.

Recommended top-level structure:

- Keep those sections in the top-level `CMakeLists.txt`.
- Within those sections, include smaller files only where doing so preserves readability.

That gives you both:

- an overview a novice can read in one pass
- smaller implementation files for the noisier details

## Priority order

If the goal is maximum complexity reduction with minimum risk, refactor in this order:

1. Clarify naming boundaries between project, package, and exported targets.
2. Remove redundant target lists and wrapper targets.
3. Simplify test registration.
4. Move docs out of the top-level file, and preferably mostly out of CMake.
5. Re-check whether environment-forwarding scripts are still needed.

That order should produce useful readability gains early without breaking the core build/install behavior.

## Expected end result

A good end state would have:

- a much shorter top-level `CMakeLists.txt`
- one clear naming model for repo name, package name, and exported targets
- standard CMake packaging
- straightforward `add_test(...)` usage wherever possible
- optional docs isolated from the core build and mostly outside CMake
- only a small number of IDE-facing convenience targets

Most importantly, a novice should be able to answer these questions by skimming the file once:

- What gets built?
- How do I link to it?
- How do I install it?
- How do I run tests?
- Which parts are optional?
