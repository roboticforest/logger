# Agent Guidance

## Principles

- Keep changes minimal and conservative, change only what is required to make the task work correctly.
- Prefer simple, clear solutions over clever or abstract ones.
- Favor readability, maintainability, and brevity, writing code a novice can follow.
- Apply "Keep It Stupid Simple" and "Don't Repeat Yourself" guidelines without over-engineering.

## General Working Rules

- Start by understanding the local code and constraints before editing.

- Do not do normal development work directly on `main` or `master`.
- If currently on `main` or `master`, create a work branch before editing or committing.
- Do not create extra branches unnecessarily when already on the correct work branch.
- Direct merges into `main` are allowed when the user explicitly asks for them.

- Plan and split work into small isolated phases:
  - Build solutions incrementally. Add the smallest useful piece, test or validate it, then continue.
  - Avoid generating a large scaffold all at once. Prefer a minimal working step over a complete-looking but unproven structure.
  - When a task needs multiple pieces, split the work into micro-steps that can each be understood, tested, and reverted independently.
  - Start with the least code that demonstrates the path forward, even if it is incomplete.

- Avoid broad refactors unless they are required to complete the task safely.
- Preserve existing structure and style unless there is a clear reason to change them.
- Reuse existing patterns when they are clear and sufficient.
- Prefer standardized, built-in, or well-established tools over custom solutions when they meet the need.
- When choosing between options, prefer the one that is easier to read, test, and maintain.
- Explain changes plainly and with minimal jargon.
- If a larger refactor seems beneficial but is not required, note it separately instead of folding it into the main change.

## Code Style

- Choose explicit, descriptive names for functions, variables, types, and files. (See [Code Naming Rules](#code-naming-rules) below.)
- Avoid unnecessary indirection, abstraction, and configuration.
- Prefer small, focused functions with clear responsibilities.

## Code Formatting Rules

- Private class members should be prefixed with an underscore.
- Use snake_case for variable and function names.
- Use PascalCase for class and type names.
- Use hyphen-case for multi-word file names.
- Use UPPER_SNAKE_CASE for global or constant names.
- Use four spaces for indentation.
- Use double quotes for string literals, and single quotes for character literals.
- Prefer `.cpp` and `.hpp` extensions, and `.cppm` for module interfaces.

## Code Naming Rules

- Prefer clear contextual variable names that describe what a value is, not where it happens to be used.
- Name functions by their observable purpose or by the data transformation they perform, whichever is clearest.

- Generally avoid abbreviations except for extremely common ones, such as: `idx`, `id`, `pos`, `val`, `var`, `str`, `args`, and `num`.
- Use suffixes only when they clarify meaning, especially for units, containers, or to avoid ambiguity.
- Do not add suffixes mechanically when the surrounding context already makes the meaning obvious.
- If using suffixes, prefer those that describe how variables are meant to be used, such as:
    - `_var` for literal variable name strings.
    - `_str` for general string variables.
    - `_len` for lengths.
    - `_dur` for durations.
- Include unit suffixes where applicable (e.g., `time_ms`, `length_ft`, `width_px`, etc.)
- If using a suffix for containers, use plural/singular suffixes for collections/elements:
  - `_items`, `_list` for list-like containers.
  - `_map`, `_dict` for hashtable-like containers.
  - `_item`, `_elem` for individual elements.
- Some examples of non-suffixed contextual names:
  - `car` from a `cars` collection.
  - `timestamp_s` from a `timeline` object.
  - `frame` from an `animation` instance.
  - `nearest_vertex` from a `find_nearest_vertex(model)` function.

## Code Documentation Rules

- Use Doxygen-style comments for all public APIs and functions.
- Keep documentation clear, concise, and up to date.
- Document purpose, parameters, return values, side effects, and important constraints where applicable.

## Inline Comment Rules

- Prefer self-documenting code so comments are needed less often.
- Inline comments should explain intent, constraints, or non-obvious decisions.
- Avoid restating what is already in the code.
- Terse scaffolding comments are acceptable.

## Project Building and Running

- Build the project using CMake and Ninja, using basic options.
- Toolchain: Visual Studio, Generator: Ninja, Build Dir: `build\debug` or `build\release`.