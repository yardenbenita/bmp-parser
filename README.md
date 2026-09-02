# BMP Parser & Vulnerability Research Lab

A defensive BMP parser written in C as a hands-on project for learning secure
binary parsing, memory safety, debugging, fuzzing, and vulnerability research
techniques.

The main BMP parser is implemented with explicit input validation and
integer-overflow checks. Separate small programs under `asan_lab/` and
`vuln_lab/` are used as controlled exercises for studying common memory-safety
bugs and unsafe arithmetic.

## Project Goals

The project was built to practice:

- Parsing a binary file format safely in C.
- Working with attacker-controlled file input.
- Validating BMP headers and metadata.
- Preventing integer overflows in size calculations.
- Investigating program execution with GDB.
- Detecting memory-safety problems with AddressSanitizer.
- Using UndefinedBehaviorSanitizer.
- Coverage-guided fuzzing with libFuzzer.
- Measuring code coverage with LLVM tools.
- Creating targeted inputs to reach security-sensitive code paths.

## Project Structure

### `src/`

Implementation of the defensive BMP parser.

### `include/`

Header files used by the parser.

### `tests/`

BMP files used to test valid input, malformed input, and edge cases.

### `fuzz/`

The libFuzzer harness used to feed generated and mutated BMP inputs into the
parser.

### `corpus/`

Inputs used by libFuzzer during fuzzing.

### `corpus_targeted/`

Manually crafted BMP inputs designed to exercise specific security-sensitive
branches that were not sufficiently reached by the existing corpus.

### `asan_lab/`

Small standalone C programs used to practice AddressSanitizer and understand
memory-safety errors, including:

- Heap buffer overflow
- Out-of-bounds read
- Out-of-bounds write
- Use-after-free

### `vuln_lab/`

Small controlled examples used to study integer-overflow and unsafe
size-calculation scenarios separately from the defensive BMP parser.

These programs are intentionally kept separate from the main parser.

## Building

Build the BMP parser:

```bash
make
```

Remove generated build files:

```bash
make clean
```

## Fuzzing

Build the fuzzing target with libFuzzer, AddressSanitizer, and
UndefinedBehaviorSanitizer:

```bash
make fuzz
```

Run libFuzzer with the existing corpus:

```bash
./fuzz_bmp corpus/
```

For example, a bounded fuzzing campaign can be run with:

```bash
./fuzz_bmp corpus/ -runs=100000
```

## Code Coverage

Build the coverage-instrumented fuzzing target:

```bash
make coverage-build
```

Run the existing corpus and collect LLVM profiling data:

```bash
make coverage-run
```

Convert the raw profiling data:

```bash
make coverage-data
```

Display the overall coverage report:

```bash
make coverage-report
```

Display source-level coverage for a specific source file:

```bash
make coverage-show FILE=src/bmp.c
```

For example:

```bash
make coverage-show FILE=src/pixel_data.c
```

## Targeted Coverage Example

Coverage analysis showed that some security-sensitive branches required very
specific input values.

One targeted BMP input was created to exercise the overflow check protecting
the calculation:

```c
padded_row_size = row_size + padding;
```

For a 24-bit BMP, a width of `0x55555555` causes the calculated `row_size` to
reach `UINT32_MAX` while one byte of padding is still required.

The parser checks the addition before performing it:

```c
if (row_size > UINT32_MAX - padding)
{
    return 1;
}
```

The targeted corpus input was then used with LLVM coverage to verify that this
overflow-protection branch was actually reached.

This demonstrates the use of coverage not only as a percentage metric, but as
a way to verify that security-relevant validation logic is exercised.

## Integer-Overflow Protection

The parser validates several size calculations before performing them,
including:

```text
width * bytes_per_pixel
row_size + padding
number_of_rows * padded_row_size
```

The fuzzing corpus and targeted inputs were used to exercise these validation
paths.

## Tools

- C
- GCC
- Clang
- Make
- GDB
- AddressSanitizer (ASan)
- UndefinedBehaviorSanitizer (UBSan)
- libFuzzer
- LLVM profiling and coverage tools
- Git

## Educational Purpose

This project was developed for hands-on study of secure C programming and
vulnerability research methodology.

The main BMP parser is defensive code. Programs in the lab directories contain
small intentionally unsafe examples used only for controlled learning and
debugging exercises.


## Roadmap

The current stage focuses on building a defensive BMP parser and practicing
vulnerability research techniques in a controlled environment.

Planned next steps:

- Select a real-world open-source target written in C/C++.
- Reproduce and analyze a previously fixed CVE.
- Identify the vulnerability's root cause.
- Study the security impact of the bug.
- Analyze the upstream patch and understand how it fixes the vulnerability.
- Use GDB and sanitizers to investigate the vulnerable behavior.
- Introduce Ghidra for binary-level analysis where appropriate.
- Apply fuzzing and coverage techniques to a real-world target.

## Educational Purpose

This project was developed for hands-on study of secure C programming and
vulnerability research methodology.

The main BMP parser is defensive code. Programs in the lab directories contain
small intentionally unsafe examples used only for controlled learning and
debugging exercises.
