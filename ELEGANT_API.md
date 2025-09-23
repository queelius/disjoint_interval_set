# Elegant Disjoint Interval Set API

## Overview

This is a completely redesigned implementation of the Disjoint Interval Set (DIS) library, following the principles of simplicity, composability, and mathematical elegance. The new API lives in the `dis` namespace and provides a clean, intuitive interface for working with intervals and interval sets.

## Key Design Principles

1. **Simplicity**: Every component does one thing exceptionally well
2. **Composability**: Operations naturally compose together
3. **Mathematical Rigor**: Models true mathematical concepts accurately
4. **Zero-Cost Abstractions**: Template-based design with compile-time optimization
5. **Multiple Expression Styles**: Support both mathematical notation and programming idioms

## Architecture

```
include/dis/
├── core/                       # Essential functionality
│   ├── interval.hpp           # Single interval implementation
│   └── disjoint_interval_set.hpp  # Set of disjoint intervals
├── io/                        # Input/Output utilities
│   ├── parser.hpp            # String DSL parser
│   └── format.hpp            # Formatting and visualization
└── dis.hpp                   # Main header file
```

## Core Features

### Intervals

```cpp
using namespace dis;

// Multiple factory methods for clarity
auto closed = real_interval::closed(0, 10);         // [0, 10]
auto open = real_interval::open(0, 10);            // (0, 10)
auto point = real_interval::point(5);              // {5}
auto unbounded = real_interval::unbounded();       // (-∞, ∞)
auto positive = real_interval::greater_than(0);    // (0, ∞)

// Rich queries
bool contains = closed.contains(5);
bool overlaps = closed.overlaps(open);
double length = closed.length();
double midpoint = closed.midpoint();

// Set operations
auto intersection = closed & open;  // or closed.intersect(open)
auto hull = closed.hull(open);     // Convex hull if possible
```

### Disjoint Interval Sets

```cpp
// Fluent construction
auto set = real_set{}
    .add(0, 10)
    .add(20, 30)
    .add(25, 35);  // Automatically merged with [20,30]

// From string notation
auto parsed = real_set::from_string("[0,10] U [20,30] U {50}");

// Set operations with multiple notations
auto union_set = set1 | set2;      // Union
auto intersection = set1 & set2;    // Intersection
auto difference = set1 - set2;      // Difference
auto symmetric = set1 ^ set2;       // Symmetric difference
auto complement = ~set1;            // Complement
```

### DIS-Specific Queries

```cpp
// Component analysis
auto span = set.span();           // Convex hull of all intervals
auto gaps = set.gaps();           // Intervals between components
size_t count = set.component_count();

// Measure operations
double measure = set.measure();       // Total length
double gap_measure = set.gap_measure(); // Total gap length
double density = set.density();       // measure / span.length
```

### Functional Operations

```cpp
// Filter intervals by predicate
auto large = set.filter([](const auto& i) {
    return i.length() > 10;
});

// Transform intervals
auto scaled = set.map([](const auto& i) {
    return real_interval::closed(
        *i.lower_bound() * 2,
        *i.upper_bound() * 2
    );
});

// Iterate with action
set.for_each([](const auto& interval) {
    std::cout << interval << '\n';
});
```

### String DSL

The parser supports mathematical notation:

```cpp
// Standard notation
"[0,10]"        // Closed interval
"(0,10)"        // Open interval
"[0,10)"        // Half-open interval
"{5}"           // Singleton
"{}"            // Empty set

// Set operations (both ASCII and Unicode)
"[0,10] U [20,30]"          // Union
"[0,10] & [20,30]"          // Intersection
"[0,10] \\ [5,15]"          // Difference
"[0,10] ^ [5,15]"           // Symmetric difference
```

### Visualization

```cpp
// Different output formats
auto str = interval_formatter<double>::format(set,
    interval_formatter<double>::Style::Mathematical);   // [0,10] U [20,30]

// ASCII visualization
std::cout << interval_formatter<double>::visualize(set, 0, 100, 60);
// Output: .....[=====]......[=====]........................
```

## Comparison with Original Implementation

### Improvements

1. **Cleaner API**: Named constructors, fluent interface, consistent naming
2. **Better Composability**: All operations return values, enabling chaining
3. **Richer Queries**: gaps(), span(), density(), measure() operations
4. **String DSL**: Parse mathematical notation directly
5. **Functional Style**: filter(), map(), for_each() operations
6. **Multiple Notations**: Support both mathematical (∪, ∩) and programming (&, |) operators
7. **Better Organization**: Clean separation of concerns with core/io/extensions

### Simplifications

1. **Single Responsibility**: Each class does one thing well
2. **Value Semantics**: Immutable operations by default
3. **Template Simplicity**: Straightforward template parameters
4. **Clear Invariants**: Automatic normalization maintains disjoint property

## Usage Examples

### Resource Scheduling

```cpp
auto room_available = real_set{}
    .add(8, 9)      // 8 AM - 9 AM
    .add(10, 12)    // 10 AM - 12 PM
    .add(14, 17);   // 2 PM - 5 PM

auto meeting = real_interval::closed(10, 11);
if (room_available.contains(meeting)) {
    std::cout << "Meeting can be scheduled\n";
}

auto utilization = room_available.measure() / room_available.span().length();
```

### Data Range Analysis

```cpp
auto valid_ranges = real_set::from_string("[0,100] U [200,300]");
auto query_range = real_interval::closed(50, 250);
auto valid_portion = valid_ranges & real_set{query_range};

std::cout << "Valid data in range: " << valid_portion.measure() << " units\n";
```

## Building and Testing

```bash
# Compile tests
g++ -std=c++20 -O2 -I./include -o test tests/test_elegant_api.cpp
./test

# Compile examples
g++ -std=c++20 -O2 -I./include -o demo examples/elegant_api_demo.cpp
./demo
```

## Future Enhancements

While the core API is complete and elegant, potential extensions include:

1. **Compile-time intervals**: Static intervals for compile-time validation
2. **Multi-dimensional support**: Hyperrectangles and boxes
3. **Custom boundary types**: Support for discrete types, dates, etc.
4. **Lazy evaluation**: For complex operation chains
5. **Parallel algorithms**: For large-scale interval processing

## Conclusion

This implementation demonstrates how a focused, well-designed API can make complex mathematical concepts accessible and enjoyable to work with. The library follows the Unix philosophy of doing one thing well, while providing multiple ways to express operations for different use cases and preferences.

The result is a library that is:
- **Simple** to understand and use
- **Powerful** enough for complex applications
- **Elegant** in its design and implementation
- **Composable** for building larger systems
- **Efficient** with zero-cost abstractions

This is the kind of API that would be worthy of inclusion in Boost or the C++ standard library.