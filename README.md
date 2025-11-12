# Disjoint Interval Set (DIS)

[![C++17](https://img.shields.io/badge/C%2B%2B-17%2F20%2F23-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![Header-Only](https://img.shields.io/badge/Header--Only-Yes-green.svg)](https://github.com/yourusername/disjoint_interval_set)
[![Test Coverage](https://img.shields.io/badge/Coverage-90.32%25-brightgreen.svg)](https://github.com/yourusername/disjoint_interval_set)
[![Core Coverage](https://img.shields.io/badge/Core%20Coverage-97.46%25-brightgreen.svg)](https://github.com/yourusername/disjoint_interval_set)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/docs-live-blue.svg)](https://yourusername.github.io/disjoint_interval_set)

> A modern C++ header-only library implementing Disjoint Interval Sets as a complete Boolean algebra, featuring an elegant STL-aligned API, compile-time interval arithmetic, and mathematical notation parsing.

## Why DIS?

Working with intervals is fundamental in many domains—from computational geometry to scheduling algorithms. While libraries like Boost.ICL provide interval containers, DIS takes a different approach by treating interval sets as first-class mathematical objects that form a complete Boolean algebra. This leads to:

- **Mathematical Elegance**: Operations follow Boolean algebra axioms rigorously
- **Intuitive API**: Express complex set operations naturally with operators
- **Zero-Cost Abstractions**: Compile-time interval validation with no runtime overhead
- **STL Compatibility**: Full alignment with C++ Standard Library conventions
- **Composability**: Features combine seamlessly without surprises
- **Production Ready**: 97.46% test coverage on core implementation, 94 test cases

## What's New (v1.1.0 - November 2024)

Major library redesign focused on **STL alignment**, **API refinement**, and **testing framework standardization**:

- **68 changes** implementing full STL container conformance
- **100% backward compatibility** through deprecated-but-functional legacy API
- **Zero breaking changes** to mathematical operators and factory methods
- **Testing framework consolidation** for improved reliability
- **All tests passing** with 97.46% core coverage

### Key Improvements

| Feature | Before | After |
|---------|--------|-------|
| **Container Methods** | `is_empty()`, `add()` | `empty()`, `insert()`, `erase()` |
| **Type Traits** | Basic | Full STL typedefs (`value_type`, `iterator`, etc.) |
| **Comparison** | Fixed `partial_ordering` | Type-aware ordering (strong for int, partial for double) |
| **Range Support** | Limited | Full C++20 ranges compatibility |
| **Iterator Interface** | Forward only | Bidirectional with reverse iterators |

See the [full STL alignment report](docs/reports/STL_ALIGNMENT_REPORT.md) for details.

## Quick Start

```cpp
#include <dis/dis.hpp>
using namespace dis;

// Create intervals with expressive factory methods
auto morning = real_interval::closed(9, 12);    // [9, 12]
auto afternoon = real_interval::closed(14, 18); // [14, 18]

// Build sets using fluent interface
auto work_hours = real_set{}
    .add(morning)
    .add(afternoon);

// Parse from mathematical notation
auto meetings = real_set::from_string("[10,11] ∪ [15,16]");

// Boolean operations with natural syntax
auto free_time = work_hours - meetings;         // Set difference
auto conflicts = work_hours & meetings;         // Intersection
bool has_conflicts = !conflicts.empty();

// Rich query interface
std::cout << "Free time: " << free_time << '\n';
std::cout << "Total free hours: " << free_time.measure() << '\n';
```

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage Examples](#usage-examples)
- [API Reference](#api-reference)
- [Testing](#testing)
- [Mathematical Foundation](#mathematical-foundation)
- [Performance](#performance)
- [Building and Testing](#building-and-testing)
- [Contributing](#contributing)
- [License](#license)

## Features

### STL-Aligned Container Interface
Full compatibility with C++ Standard Library conventions: `empty()`, `insert()`, `erase()`, `clear()`, reverse iterators, and proper type traits.

### Complete Boolean Algebra
Full support for union, intersection, complement, difference, and symmetric difference with proper algebraic properties.

### Mathematical Notation Parser
Parse interval sets from strings using standard mathematical notation: `"[0,5) ∪ (10,20] ∪ {25}"`.

### Compile-Time Interval Arithmetic
Zero-overhead interval bounds checking at compile-time using template metaprogramming.

### Rich Query Interface
Comprehensive set of predicates and queries: gaps, span, density, measure, overlaps, and more.

### Multiple Expression Styles
Support both mathematical operators (`|`, `&`, `~`) and explicit methods (`unite()`, `intersect()`, `complement()`).

### Production Ready
Extensively tested with 90.32% overall coverage and 97.46% coverage on core implementation.

## Installation

DIS is a header-only library. Simply clone and include:

```bash
git clone https://github.com/yourusername/disjoint_interval_set.git
```

```cpp
// Option 1: Include everything
#include <dis/dis.hpp>

// Option 2: Include only what you need
#include <dis/core/interval.hpp>
#include <dis/core/disjoint_interval_set.hpp>
```

### Requirements

#### Library Usage
- **C++17 or later** (uses `std::optional`, structured bindings)
- **C++20 recommended** (for ranges and concepts)
- **No external dependencies** for library usage
- **Tested compilers**: GCC 9+, Clang 10+, MSVC 2019+

#### Building and Testing (Optional)
- **CMake 3.14+** for build system
- **Google Test** for running the test suite
- **gcovr** (optional) for HTML coverage reports: `pip install gcovr`

## Usage Examples

### Basic Intervals

```cpp
#include <dis/dis.hpp>
using namespace dis;

// Multiple factory methods for clarity
auto closed = real_interval::closed(0, 10);         // [0, 10]
auto open = real_interval::open(0, 10);            // (0, 10)
auto left_open = real_interval::left_open(0, 10);  // (0, 10]
auto right_open = real_interval::right_open(0, 10); // [0, 10)
auto point = real_interval::point(5);              // {5}
auto empty = interval<double>{};                   // ∅

// Unbounded intervals
auto positive = real_interval::greater_than(0);    // (0, ∞)
auto non_negative = real_interval::at_least(0);    // [0, ∞)

// Interval operations
auto i1 = real_interval::closed(0, 10);
auto i2 = real_interval::closed(5, 15);

auto intersection = i1 & i2;  // [5, 10]
auto hull = i1.hull(i2);      // [0, 15]

// Queries
assert(i1.contains(5));
assert(i1.overlaps(i2));
assert(i1.length() == 10);
assert(i1.midpoint() == 5);
```

### Disjoint Interval Sets

```cpp
// Build sets with fluent interface
auto set = real_set{}
    .add(0, 10)      // Add [0, 10]
    .add(20, 30)     // Add [20, 30]
    .add(25, 35);    // Automatically merged to [20, 35]

// STL-compatible insertion
real_set ranges;
ranges.insert(real_interval::closed(0, 5));
ranges.insert(real_interval::closed(10, 15));

// From initializer list
real_set ranges2 = {
    real_interval::closed(0, 5),
    real_interval::closed(10, 15),
    real_interval::closed(12, 20)  // Overlaps merged automatically
};

// Set operations with multiple notations
auto a = real_set::from_string("[0,10] ∪ [20,30]");
auto b = real_set::from_string("[5,15] ∪ [25,35]");

auto union_set = a | b;          // [0, 15] ∪ [20, 35]
auto intersection = a & b;       // [5, 10] ∪ [25, 30]
auto difference = a - b;         // [0, 5) ∪ (10, 20) ∪ (30, 35]
auto symmetric = a ^ b;          // Symmetric difference
auto complement = ~a;            // Complement

// Advanced queries
auto gaps = set.gaps();          // Intervals between components
auto span = set.span();          // Smallest interval containing all
auto density = set.density();    // measure / span.length
auto measure = set.measure();    // Total length covered

// Functional operations
set.for_each([](const auto& interval) {
    std::cout << interval << '\n';
});

auto filtered = set.filter([](const auto& i) {
    return i.length() > 5;
});
```

### String DSL

```cpp
// Parse from mathematical notation
auto set = real_set::from_string("[0,5) ∪ (10,20] ∪ {25} ∪ [30,∞)");

// Alternative notation support
auto set2 = real_set::from_string("[0,5) U (10,20] U {25}");  // 'U' for union
auto set3 = real_set::from_string("[0,5), (10,20], {25}");    // Comma separated

// Round-trip formatting
std::string notation = set.to_string();
assert(real_set::from_string(notation) == set);

// Custom formatting
std::cout << set.to_string(format_options::unicode) << '\n';    // Uses ∪, ∩, ∅
std::cout << set.to_string(format_options::ascii) << '\n';      // Uses U, ^, {}
```

### Compile-Time Intervals

```cpp
#include <dis/static_interval.hpp>

// Define compile-time intervals
template<int Min, int Max>
using valid_range = dis::static_interval<int, Min, Max, false, false>;

// Compile-time validation
using age_range = valid_range<0, 150>;
using percentage_range = valid_range<0, 100>;

static_assert(age_range::contains(25), "25 is a valid age");
static_assert(!age_range::contains(200), "200 is not a valid age");

// Build bounded types with zero runtime overhead
template<int Min, int Max>
class bounded {
    using bounds = valid_range<Min, Max>;
    int value_;
public:
    constexpr bounded(int v) : value_(v) {
        if (!bounds::contains(v))
            throw std::out_of_range("Value out of bounds");
    }
    constexpr operator int() const { return value_; }
};

using age = bounded<0, 150>;
using percentage = bounded<0, 100>;
```

### STL Algorithm Compatibility

```cpp
real_set set = /* ... */;

// STL algorithms work seamlessly
auto it = std::find_if(set.begin(), set.end(),
    [](const auto& i) { return i.length() > 10; });

auto count = std::count_if(set.begin(), set.end(),
    [](const auto& i) { return i.contains(50); });

// Reverse iteration
for (auto it = set.rbegin(); it != set.rend(); ++it) {
    std::cout << *it << '\n';
}

// C++20 ranges
auto long_intervals = set | std::views::filter(
    [](const auto& i) { return i.length() > 100; });

auto lengths = set | std::views::transform(
    [](const auto& i) { return i.length(); });
```

## API Reference

### Core Classes

#### `interval<T>`

Represents a single interval with configurable boundaries.

**Key Methods:**
- `empty()` - Test if interval is empty
- `contains(T value)` - Test membership
- `overlaps(interval other)` - Test overlap
- `length()` - Get interval length
- `hull(interval other)` - Compute convex hull
- `intersect(interval other)` - Compute intersection

**Factory Methods:**
```cpp
interval::closed(a, b)       // [a, b]
interval::open(a, b)         // (a, b)
interval::left_open(a, b)    // (a, b]
interval::right_open(a, b)   // [a, b)
interval::point(x)           // {x}
interval::unbounded()        // (-∞, ∞)
interval::greater_than(x)    // (x, ∞)
interval::at_least(x)        // [x, ∞)
interval::less_than(x)       // (-∞, x)
interval::at_most(x)         // (-∞, x]
```

#### `disjoint_interval_set<IntervalType>`

Maintains a set of non-overlapping intervals with Boolean algebra operations.

**STL Container Interface:**
- `empty()` - Test if set is empty
- `size()` - Number of disjoint intervals
- `insert(interval)` - Add interval (merges if needed)
- `erase(iterator)` - Remove interval by iterator
- `erase(interval)` - Remove specific interval
- `clear()` - Remove all intervals
- `swap(other)` - Swap contents
- `begin()`, `end()` - Forward iterators
- `rbegin()`, `rend()` - Reverse iterators
- `front()`, `back()` - Access first/last interval

**Set Operations:**
- `unite(other)` / `operator|` - Union
- `intersect(other)` / `operator&` - Intersection
- `complement()` / `operator~` - Complement
- `difference(other)` / `operator-` - Set difference
- `symmetric_difference(other)` / `operator^` - Symmetric difference

**Query Operations:**
- `contains(value)` - Test value membership
- `contains(interval)` - Test interval subset
- `measure()` - Total length covered
- `span()` - Smallest containing interval
- `gaps()` - Intervals between components
- `density()` - Measure / span ratio

**Type Aliases:**
```cpp
using value_type = typename I::value_type;
using size_type = std::size_t;
using iterator = const_iterator;
using reverse_iterator = std::reverse_iterator<const_iterator>;
// ... and more STL-standard aliases
```

### Type Aliases

```cpp
namespace dis {
    // Common interval types
    using real_interval = interval<double>;
    using integer_interval = interval<int>;

    // Common set types
    using real_set = disjoint_interval_set<real_interval>;
    using integer_set = disjoint_interval_set<integer_interval>;
}
```

## Testing

### Test Coverage

Current test coverage (as of November 2024):

| Component | Coverage | Status |
|-----------|----------|--------|
| **Core Implementation** | **97.46%** | Excellent |
| `disjoint_interval_set.hpp` | 97.46% (537/551 lines) | Production-ready |
| `interval.hpp` | 89.53% (479/535 lines) | Well-tested |
| **Overall Library** | **90.32%** | Very good |
| `parser.hpp` | 60.33% | Good |
| `format.hpp` | 24.90% | Basic coverage |

### Test Suite Organization

- **94 test cases** covering all major functionality
- **500+ assertions** ensuring correctness
- **~2,400 lines** of test code

**Test Categories:**

1. **Comprehensive Interval Tests** (22 test cases)
   - Construction, boundaries, containment
   - Set operations (intersection, hull)
   - Measures (length, midpoint, distance)
   - Edge cases (infinity, NaN, extremes)

2. **Comprehensive DIS Tests** (32 test cases)
   - Construction and initialization
   - Set operations (union, intersection, etc.)
   - Queries (contains, overlaps, gaps)
   - Functional operations (filter, map, for_each)
   - Performance with 1000+ intervals

3. **Elegant API Tests** (14 test cases)
   - Fluent interface patterns
   - Real-world usage scenarios
   - Integration tests

4. **Parser & Formatter Tests** (26 test cases)
   - Various interval notations
   - Round-trip parsing
   - Output formatting styles

### Running Tests

#### Prerequisites
First, install Google Test:

```bash
# Ubuntu/Debian
sudo apt-get install libgtest-dev cmake

# macOS with Homebrew
brew install googletest

# Or build from source
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Build and Test

```bash
# Clone the repository
git clone https://github.com/yourusername/disjoint_interval_set.git
cd disjoint_interval_set

# Build and run tests
mkdir build && cd build
cmake ..
make
ctest --verbose

# Or use the convenience target
make run_tests

# Generate coverage report (requires gcovr: pip install gcovr)
make coverage
# Report opens at: build/coverage/index.html
```

See the [full test coverage report](docs/reports/TEST_COVERAGE_REPORT.md) for detailed metrics.

## Mathematical Foundation

### Boolean Algebra

The Disjoint Interval Set forms a complete Boolean algebra where:

- **Elements**: Sets of disjoint intervals
- **Join (∨)**: Union operation (`operator|`)
- **Meet (∧)**: Intersection operation (`operator&`)
- **Complement (¬)**: Complement operation (`operator~`)
- **Bottom (⊥)**: Empty set
- **Top (⊤)**: Universal set

### Axioms Satisfied

1. **Associativity**: `(A ∪ B) ∪ C = A ∪ (B ∪ C)` and `(A ∩ B) ∩ C = A ∩ (B ∩ C)`
2. **Commutativity**: `A ∪ B = B ∪ A` and `A ∩ B = B ∩ A`
3. **Distributivity**: `A ∩ (B ∪ C) = (A ∩ B) ∪ (A ∩ C)`
4. **Identity**: `A ∪ ∅ = A` and `A ∩ U = A`
5. **Complement**: `A ∪ ~A = U` and `A ∩ ~A = ∅`
6. **Idempotence**: `A ∪ A = A` and `A ∩ A = A`
7. **Absorption**: `A ∪ (A ∩ B) = A` and `A ∩ (A ∪ B) = A`
8. **De Morgan's Laws**: `~(A ∪ B) = ~A ∩ ~B` and `~(A ∩ B) = ~A ∪ ~B`

### Why Disjoint?

Maintaining disjoint (non-overlapping) intervals provides:

1. **Canonical Form**: Unique representation for each set
2. **Efficient Operations**: O(n + m) for most set operations
3. **Space Efficiency**: No redundant interval storage
4. **Clear Semantics**: Each point belongs to at most one interval

## Performance

### Complexity Analysis

| Operation | Complexity | Notes |
|-----------|------------|-------|
| `insert(interval)` | O(n) | n = number of intervals |
| `contains(value)` | O(log n) | Binary search |
| `union (A ∪ B)` | O(n + m) | Linear merge |
| `intersection (A ∩ B)` | O(n + m) | Linear scan |
| `complement (~A)` | O(n) | Invert boundaries |
| `measure()` | O(n) | Sum all lengths |
| `parse(string)` | O(s) | s = string length |

### Memory Usage

- Each interval: 2 × sizeof(T) + 2 bits for boundaries
- Set overhead: std::vector overhead + bookkeeping
- No memory allocation for compile-time intervals

### Optimization Techniques

1. **Move Semantics**: Full support for efficient transfers
2. **Small Vector Optimization**: For sets with few intervals
3. **Lazy Evaluation**: Operations can be composed without materialization
4. **Compile-Time Computation**: Zero runtime cost for static intervals

## Comparison with Alternatives

### vs Boost.ICL (Interval Container Library)

| Feature | DIS | Boost.ICL |
|---------|-----|-----------|
| **Philosophy** | Mathematical Boolean algebra | Container-centric |
| **API Style** | Natural operators (`|`, `&`, `~`) | Method calls |
| **String Parsing** | Built-in DSL | Not available |
| **Compile-Time** | Full support | Limited |
| **Dependencies** | None | Boost |
| **Header-Only** | Yes | Yes |
| **STL Alignment** | Full (v2.0) | Partial |
| **Learning Curve** | Intuitive | Steeper |

### vs std::set<std::pair<T,T>>

| Feature | DIS | std::set |
|---------|-----|----------|
| **Interval Merging** | Automatic | Manual |
| **Set Operations** | O(n + m) | O(n × m) worst case |
| **Mathematical Operations** | Native | Must implement |
| **Memory Usage** | Optimized | Higher overhead |
| **Type Safety** | Interval types | Raw pairs |

## Building and Testing

### Building Examples

```bash
# Compile examples
g++ -std=c++17 -I./include examples/elegant_api_demo.cpp -o demo
./demo

# With optimizations
g++ -std=c++20 -O3 -I./include examples/scheduling_demo.cpp -o scheduling
./scheduling
```

### Documentation

Build the documentation site:

```bash
# Install mkdocs if needed
pip install mkdocs mkdocs-material

# Serve locally
mkdocs serve

# Build static site
mkdocs build
```

Visit the [live documentation](https://yourusername.github.io/disjoint_interval_set) for comprehensive guides and API reference.

## Applications

### Real-World Use Cases

- **Computational Geometry**: Polygon clipping, CSG operations
- **Scheduling Systems**: Resource allocation, conflict detection
- **Numerical Analysis**: Interval arithmetic, error bounds
- **Access Control**: Time-based permissions, IP range filtering
- **Data Visualization**: Histogram binning, range queries
- **Signal Processing**: Frequency band allocation
- **Game Development**: Collision detection, spatial indexing
- **Database Systems**: Range partitioning, index optimization

## Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Write tests for your changes
4. Ensure all tests pass with good coverage
5. Update documentation as needed
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### Code Style

- Follow modern C++ best practices
- Use descriptive names
- Document public APIs
- Write comprehensive tests
- Keep commits atomic and well-described

### Testing Guidelines

- Write tests for all new features
- Ensure edge cases are covered
- Maintain or improve coverage metrics (aim for 95%+)
- Test compile-time features with `static_assert`
- Run the full test suite before submitting PRs

## Documentation

### Resources

- [Quick Start Guide](docs/quickstart.md)
- [API Reference](docs/api-reference.md)
- [STL Alignment Report](docs/reports/STL_ALIGNMENT_REPORT.md)
- [Test Coverage Report](docs/reports/TEST_COVERAGE_REPORT.md)
- [Elegant API Design](docs/reports/ELEGANT_API.md)
- [Technical Report](TECHNICAL_REPORT.md)

## Roadmap

### Planned Features

- [ ] C++20 concepts for better error messages
- [ ] Parallel algorithms for large sets
- [ ] Persistent data structures
- [ ] Interval tree indexing for very large sets
- [ ] Python bindings
- [ ] Formal verification of algebraic properties
- [ ] Integration with computational geometry libraries

### Under Consideration

- [ ] PMR allocator support (if user demand exists)
- [ ] Custom execution policies
- [ ] Boost library submission

**Note**: Multi-dimensional interval support was removed in v1.1.0 to focus the library on 1D interval operations with maximum quality and performance.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by mathematical interval theory and Boolean algebra
- Thanks to the C++ community for feedback and suggestions
- Special thanks to contributors and early adopters
- Boost.ICL for pioneering interval containers in C++

## Citation

If you use this library in academic work, please cite:

```bibtex
@software{dis2024,
  title = {Disjoint Interval Set: A Boolean Algebra for C++},
  author = {Your Name},
  year = {2024},
  url = {https://github.com/yourusername/disjoint_interval_set}
}
```

---

<p align="center">
  <i>Elegant interval arithmetic for modern C++</i><br>
  <a href="https://yourusername.github.io/disjoint_interval_set">Documentation</a> •
  <a href="https://github.com/yourusername/disjoint_interval_set/issues">Issues</a> •
  <a href="CONTRIBUTING.md">Contributing</a>
</p>
