# Agent Guidance

## Principles

- Keep changes minimal and conservative, change only what is required to make the task work correctly.
- Prefer simple, clear solutions over clever or abstract ones.
- Favor readability, maintainability, and brevity, writing code a novice can follow.
- Apply "Keep It Stupid Simple" and "Don't Repeat Yourself" guidelines without over-engineering.

## General Working Rules

- Start by understanding the local code and constraints before editing.
- Preserve existing structure and style unless there is a clear reason to change them.
- Avoid broad refactors unless they are required to complete the task safely.
- Reuse existing patterns when they are clear and sufficient.
- Prefer standardized, built-in, or well-established tools over custom solutions when they meet the need.
- When choosing between options, prefer the one that is easier to read, test, and maintain.
- Explain changes plainly and with minimal jargon.
- If a larger refactor seems beneficial but is not required, note it separately instead of folding it into the main change.

## Code Style

- Choose explicit, descriptive names for functions, variables, types, and files.
- Avoid unnecessary indirection, abstraction, and configuration.
- Prefer small, focused functions with clear responsibilities.

## Code Formatting Rules

- Private class members should be prefixed with an underscore.
- Use snake_case for variable and function names.
- Use PascalCase for class and type names.
- Use hyphen-case for multi-word file names.
- Use four spaces for indentation.
- Prefer `.cpp` and `.hpp` extensions, and `.cppm` for module interfaces.

## Code Documentation Rules

- Use Doxygen-style comments for all public APIs and functions.
- Keep documentation clear, concise, and up to date.
- Document purpose, parameters, return values, side effects, and important constraints where applicable.

## Inline Comment Rules

- Prefer self-documenting code so comments are needed less often.
- Inline comments should explain intent, constraints, or non-obvious decisions.
- Avoid restating what is already in the code.

## Project Building and Running

- Build the project using CMake and Ninja, using basic options.
- Toolchain: Visual Studio, Generator: Ninja, Build Dir: `build\debug` or `build\release`.