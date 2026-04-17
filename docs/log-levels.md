\page log_levels Log Levels

This page defines the semantic intent of each logging level used by `DV::Logger`.
The same guidance applies to public and internal documentation.

## Level meanings

- `INFO` is general-purpose status information and normal operation reporting.
- `WARN` signals potential problems or risky operations that may still be recoverable.
- `ERROR` records definite failures (for example, missing resources or failed operations).
- `FATAL` records unrecoverable conditions from the application's perspective.
- `DEBUG` is for detailed implementation diagnostics (state, values, addresses, etc.).
- `TRACE` is for high-frequency flow/call-tracing style diagnostics.

## Notes

- Calling `fatal()` does **not** terminate the process; it only emits a `FATAL` entry.
- `trace()` does not inspect call stacks automatically; it only labels entries as `TRACE`.
- Future filtering by level may be introduced, but all levels currently emit unconditionally.
