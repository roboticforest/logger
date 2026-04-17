\page api_reference API Reference

This page is the stable user-facing API reference for the `logger` module.
It exists because some Doxygen versions do not reliably extract full class/member docs
from C++20 module interface units (`.cppm`).

## Namespace

- `DV`

## Class: `DV::Logger`

A simple logging utility built around `std::ostream`.
It writes timestamped entries with a logger name and level tag.

### Construction and setup

- `explicit Logger(const char* name, std::ostream& os)`
- `void addSplit(std::ostream& os)`

### Copy/move behavior

- Copy and move are disabled.

### Logging methods

- `debug(...)`
- `error(...)`
- `fatal(...)`
- `info(...)`
- `trace(...)`
- `warn(...)`

Each logging call emits one entry. Arguments are stream-formatted and joined with spaces.
Level semantics are documented in @ref log_levels.

## Version info

`DV::Logger::Version` provides:

- `string`
- `major`
- `minor`
- `patch`
- `tweak`
