# Repository Guidelines

## Project Structure & Module Organization
This is a small C++ library focused on deterministic math and dice/roll utilities.
- `dice.cpp`, `dmath.cpp` and their headers (`dice.h`, `dmath.h`, `direction.h`, `fixp.h`, `perten.h`) contain the core library code.
- `tests/` holds unit tests and benchmarks (e.g., `tests/test1.cpp`, `tests/perf_roll.cpp`).
- Build output is typically generated into `build/` or a custom directory via CMake.

## Build, Test, and Development Commands
Use CMake out-of-tree builds:
```sh
cmake -S . -B build
cmake --build build
```
Run the test suite with CTest:
```sh
ctest --test-dir build
```
You can run individual executables from the build directory, for example:
`./build/test1` or `./build/perf_roll`.

## Coding Style & Naming Conventions
- Indentation uses tabs (see existing `.cpp` files).
- Prefer C++ headers with `.h` and sources with `.cpp`.
- Types and functions use lower_snake_case or lowerCamelCase (match existing headers), and enums use `enum class` (e.g., `luck_type`).
- There is no enforced formatter in the repo; keep style consistent with neighboring code.

## Testing Guidelines
- Tests and benchmarks are compiled as standalone executables via CMake and registered with CTest.
- Test files follow `tests/test_*.cpp` or `tests/*_test.cpp` patterns (see `tests/`).
- Add or update tests alongside behavior changes, then run `ctest --test-dir build`.

## Commit & Pull Request Guidelines
- Recent commits use short, imperative, sentence-case messages (e.g., “Make derive function output more varied”).
- Keep commits focused; describe what changed and why.
- PRs should include a brief summary, testing performed (commands + results), and any relevant context or tradeoffs.
