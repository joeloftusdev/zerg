# Logger

## Features:
 * Asynchronous Logging: Uses a separate thread to process log entries, improving performance by offloading I/O operations.
 * Circular Buffer: Log entries are stored in a circular buffer, reducing the overhead of frequent file writes.
 * Type-safe: Formatting is done via fmtlib.
 * Safe: No references taken to arguments unless explicitly requested.
 * Suite of Unit Tests: Runs cleanly under AddressSanitizer, UndefinedBehaviourSanitizer, ThreadSanitizer, and LeakSanitizer.
 * File and Line Information: Retrieves the file and line number where the log message was received.
 * Configurable: Verbosity and file location can be configured via a configuration file.

   

![CI](https://github.com/joeloftusdev/cpp_logger/actions/workflows/ubuntu_clang.yml/badge.svg)
![CI](https://github.com/joeloftusdev/cpp_logger/actions/workflows/ubuntu_gcc.yml/badge.svg)
