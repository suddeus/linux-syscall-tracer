# Linux System Call Tracer

A small Linux command-line tracer built with `ptrace(2)`. It can start a command under tracing or attach to an existing process, then prints the system calls it observes.

> **Architecture support:** Linux **x86-64** and **AArch64**.

## Features

- Trace a newly started command or attach to a process by PID.
- Send output to standard output or a file.
- Filter the trace to known syscall names.
- Decode a small set of arguments, including pathname strings, open flags, modes, and directory file descriptors.

The built-in syscall table currently recognizes:

| Architecture | System calls |
| --- | --- |
| x86-64 | `read`, `write`, `open`, `close` |
| AArch64 | `read`, `write`, `openat`, `close` |

Other system calls are emitted as `Unknown syscall[NUMBER](...)` with hexadecimal register values.

## Requirements

- Linux on x86-64 or AArch64
- A C++23 compiler
- CMake 3.28 or newer
- Permission to trace the target process. Attaching to another process may be restricted by Linux `ptrace` security settings or by the target's ownership.

## Build

```bash
cmake -S . -B build
cmake --build build
```

The executable is written to `build/syscall_tracer`.

## Usage

```text
syscall_tracer [options] command [args...]
syscall_tracer [options] -p PID
```

Options:

| Option | Description |
| --- | --- |
| `-h`, `--help` | Show the command help. |
| `-p`, `--pid PID` | Attach to an existing process instead of starting a command. |
| `-e LIST` | Trace only the named, comma-separated syscalls. |
| `-o`, `--output FILE` | Write the trace to `FILE` instead of standard output. |

### Examples

Trace a command:

```bash
./build/syscall_tracer /bin/echo hello
```

Trace only writes and save them to a file:

```bash
./build/syscall_tracer -e write -o trace.log /bin/echo hello
```

Attach to a running process:

```bash
./build/syscall_tracer -p 12345
```

## Project layout

```text
main.cpp                 Program entry point
include/config.hpp       Command-line configuration
include/child_starter.hpp Process launch/attach abstraction
include/syscalls.hpp     Syscall metadata and argument formatters
include/tracer.hpp       Tracing interface
src/                     Implementations
```
