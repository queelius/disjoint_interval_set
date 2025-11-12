# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This is a modern C++ header-only library implementing **Disjoint Interval Sets (DIS)** as a complete Boolean algebra over 1-dimensional intervals. The library provides set-theoretic operations (union, intersection, complement) with an elegant API that balances mathematical notation with STL conventions.

**Key characteristics:**
- Header-only with **zero dependencies**
- C++17 minimum, C++20 for full features
- 97.46% test coverage on core implementation
- STL-aligned API (post-refactoring in Nov 2024)
- Production-ready with comprehensive test suite

## Dual Implementation Architecture

The library maintains **two implementations** for different use cases:

### 1. Modern API (`include/dis/`)
**Primary implementation going forward** - STL-aligned, elegant, feature-complete.

```
include/dis/
├── core/
│   ├── interval.hpp              # Core interval class with STL compliance
│   └── disjoint_interval_set.hpp # Main DIS container with full Boolean algebra
├── io/
│   ├── parser.hpp               # Mathematical notation parser
│   └── format.hpp               # Output formatting
└── dis.hpp                      # Convenience header (includes all)
```

**Key features:**
- STL container interface (`empty()`, `size()`, `insert()`, `erase()`, `clear()`, `swap()`)
- Mathematical operators (`|`, `&`, `~`, `-`, `^`) for Boolean algebra
- Named constructors: `interval::closed()`, `::open()`, `::left_open()`, `::right_open()`, `::point()`
- Fluent interface: `.add()`, `.remove()` for chaining
- String DSL: `real_set::from_string("[0,10) ∪ [20,30]")`
- Full iterator support including reverse iterators
- C++20 ranges compatibility

### 2. Legacy API (`include/disjoint_interval_set/`)
**Original implementation** - kept for backward compatibility, feature-complete but older conventions.

Contains: `interval.hpp`, `disjoint_interval_set.hpp`, `disjoint_interval_set_algorithms.hpp`, `static_interval.hpp`, plus experimental `_v2.hpp` variants.

## Building and Testing

### Prerequisites
- **C++17 compiler** (GCC 9+, Clang 10+, MSVC 2019+)
- **Google Test** (required for running tests)
- **gcovr** (optional, for HTML coverage reports: `pip install gcovr`)
- **CMake 3.14+** (for build system)

### Quick Build (CMake)
```bash
mkdir build && cd build
cmake ..
make
```

### Run All Tests
```bash
# Using CMake (recommended)
cd build
make run_tests

# Or directly with ctest
ctest --output-on-failure --verbose

# Run specific test executable
./test_interval
./test_dis_comprehensive
./test_elegant_api
```

### Test Coverage Report
```bash
# Build with coverage enabled (requires gcovr)
mkdir build-coverage && cd build-coverage
cmake -DENABLE_COVERAGE=ON ..
make run_tests

# Generate HTML coverage report
make coverage
# Opens: build-coverage/coverage/index.html
```

### Manual Compilation (No Build System)
```bash
# Compile against modern API
g++ -std=c++17 -I./include -o my_test my_test.cpp

# With optimizations
g++ -std=c++20 -O3 -I./include -o my_test my_test.cpp

# Single test file (requires Google Test)
g++ -std=c++17 -I./include tests/test_elegant_api.cpp -o test_elegant -lgtest -lgtest_main -lpthread && ./test_elegant
```

## Test Suite Organization

**Total: 94+ test cases across 8 test files**

| Test File | Focus | Test Count | Coverage |
|-----------|-------|------------|----------|
| `test_interval_comprehensive.cpp` | Interval class operations | 22 | ~99% |
| `test_dis_comprehensive.cpp` | DIS container operations | 32 | ~98% |
| `test_elegant_api.cpp` | Modern API integration | 14 | ~98% |
| `test_parser_formatter_comprehensive.cpp` | String DSL I/O | 26 | ~88% |
| `test_interval.cpp` | Legacy interval tests | - | - |
| `test_disjoint_interval_set.cpp` | Legacy DIS tests | - | - |
| `test_algorithms.cpp` | Algorithm tests | - | - |
| `test_integration.cpp` | Cross-feature tests | - | - |

**Run specific test category:**
```bash
./build/test_interval_comprehensive    # Interval operations
./build/test_dis_comprehensive         # DIS operations
./build/test_elegant_api               # Modern API usage
```

## Core Concepts

### Interval Types
Intervals support four boundary configurations: `[a,b]`, `(a,b)`, `[a,b)`, `(a,b]`

**Creation via named constructors (preferred):**
```cpp
auto closed = real_interval::closed(0, 10);      // [0, 10]
auto open = real_interval::open(0, 10);          // (0, 10)
auto left_open = real_interval::left_open(0, 10); // (0, 10]
auto point = real_interval::point(5);            // {5}
```

### Disjoint Interval Sets
**Canonical form**: Sorted, non-overlapping, maximally merged intervals.

**Three equivalent API styles:**
```cpp
// 1. Mathematical operators (concise)
auto result = (a | b) & ~c;

// 2. Named methods (explicit)
auto result = a.unite(b).intersect(c.complement());

// 3. Fluent interface (chainable)
auto result = real_set{}.add(0, 10).add(20, 30).remove(interval::closed(5, 25));
```

### Type Aliases
```cpp
namespace dis {
    using real_interval = interval<double>;
    using integer_interval = interval<int>;
    using real_set = disjoint_interval_set<real_interval>;
    using integer_set = disjoint_interval_set<integer_interval>;
}
```

## STL Alignment (November 2024 Refactor)

The library underwent major STL alignment in Nov 2024. Key changes:

### Container Interface Compliance
- ✅ `empty()` instead of `is_empty()`
- ✅ `insert()` / `erase()` alongside legacy `add()` / `remove()`
- ✅ `front()`, `back()`, `clear()`, `swap()`
- ✅ All standard typedefs: `value_type`, `size_type`, `iterator`, `const_iterator`, `reverse_iterator`
- ✅ Rule of Five explicitly defaulted
- ✅ `operator<=>` with correct ordering categories

### Backward Compatibility
**100% backward compatible** - Legacy methods still work:
```cpp
set.add(interval);      // Still works
set.insert(interval);   // New STL-compatible way
set.is_empty();         // Deprecated warning, but works
set.empty();            // Preferred STL way
```

### C++20 Ranges Support
```cpp
// Works with range adaptors
auto filtered = my_set | std::views::filter([](auto& i) { return i.length() > 5; });

// Range algorithms
std::ranges::for_each(my_set, [](const auto& interval) { /* ... */ });
```

## Documentation

### Generated Documentation (MkDocs)
```bash
# Install dependencies
pip install mkdocs mkdocs-material pymdown-extensions mkdocs-minify-plugin

# Local development server (auto-reloads on changes)
mkdocs serve
# Opens: http://127.0.0.1:8000

# Build static site (outputs to site/ directory)
mkdocs build

# Deploy to GitHub Pages (requires push access)
mkdocs gh-deploy
```

**Live docs:** https://queelius.github.io/disjoint_interval_set/

**Note:** The `site/` directory is git-ignored and should not be committed.

### Key Documentation Files
- `README.md` - Main library documentation with usage examples
- `TECHNICAL_REPORT.md` - Academic-style technical report
- `STL_ALIGNMENT_REPORT.md` - Details of Nov 2024 STL refactoring
- `TEST_COVERAGE_REPORT.md` - Coverage analysis
- `docs/` - MkDocs source (deployed to GitHub Pages)

## Design Principles

### Mathematical Rigor
- Operations satisfy Boolean algebra axioms (associativity, commutativity, distributivity, De Morgan's laws)
- Canonical form ensures unique representation
- All boundary conditions handled correctly (open/closed endpoints)

### Zero-Cost Abstractions
- Everything is `inline` and `constexpr` where possible
- No virtual functions, no RTTI
- Move semantics throughout
- Compile-time intervals available via `static_interval.hpp`

### API Philosophy
1. **Simplicity**: Each component does one thing well
2. **Composability**: Operations chain naturally
3. **Multiple notations**: Support both mathematical (`|`, `&`) and STL (`unite()`, `intersect()`)
4. **Discoverability**: Named constructors make intent clear

## Common Development Patterns

### Adding New Interval Operations
1. Add to `include/dis/core/interval.hpp`
2. Mark `[[nodiscard]]` and `constexpr` where applicable
3. Add tests to `tests/test_interval_comprehensive.cpp`
4. Run coverage: `make coverage` and verify

### Adding New Set Operations
1. Add to `include/dis/core/disjoint_interval_set.hpp`
2. Maintain canonical form (sorted, disjoint, merged)
3. Provide operator overload AND named method
4. Add tests to `tests/test_dis_comprehensive.cpp`

### Parser/Formatter Changes
Files: `include/dis/io/parser.hpp`, `include/dis/io/format.hpp`
Tests: `tests/test_parser_formatter_comprehensive.cpp`

The parser uses recursive descent for `"[0,10) ∪ [20,30]"` notation.

## Known Limitations

1. **No multi-dimensional support** - Removed in Nov 2024. Library focuses on 1D intervals only.
2. **Parser limitations** - Does not support set operations in string format (only unions)
3. **No custom allocators** - Not an STL container in the strictest sense
4. **Floating-point precision** - Standard IEEE 754 caveats apply

## Language Requirements

- **Minimum**: C++17 (requires `std::optional`, structured bindings)
- **Recommended**: C++20 (for full ranges support and concepts)
- **Tested compilers**: GCC 9+, Clang 10+, MSVC 2019+

## Repository Structure Notes

- `examples/` - Example programs demonstrating library usage
- `tests/` - Comprehensive test suite (94+ test cases)
- `include/` - Header files (both modern and legacy implementations)
- `docs/` - MkDocs documentation source
- `site/` - Generated documentation (git-ignored)
- `build/`, `build-coverage/` - Build artifacts (git-ignored)
