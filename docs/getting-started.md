# Getting Started

This guide will help you get up and running with the Disjoint Interval Set (DIS) library in minutes.

## Installation

DIS is a header-only library with **zero dependencies**. Installation is as simple as copying the headers to your project.

### Method 1: Direct Include

```bash
# Clone the repository
git clone https://github.com/yourusername/disjoint_interval_set.git

# Copy headers to your project
cp -r disjoint_interval_set/include/dis /path/to/your/project/include/
```

### Method 2: Submodule

```bash
# Add as git submodule
git submodule add https://github.com/yourusername/disjoint_interval_set.git external/dis

# In your CMakeLists.txt
include_directories(external/dis/include)
```

### Method 3: System-Wide Installation

```bash
# Install to /usr/local/include
sudo cp -r include/dis /usr/local/include/
```

## Requirements

- **C++17 or later** (uses `std::optional`, structured bindings)
- **No external dependencies**
- Tested on:
  - GCC 9+
  - Clang 10+
  - MSVC 2019+
  - Apple Clang 12+

## Your First Program

Create a file `example.cpp`:

```cpp
#include <dis/dis.hpp>
#include <iostream>

using namespace dis;

int main() {
    // Create intervals using named constructors
    auto morning = real_interval::closed(9, 12);
    auto afternoon = real_interval::closed(14, 17);

    // Build a set using fluent interface
    auto work_hours = real_set{}
        .add(morning)
        .add(afternoon);

    // Test membership
    if (work_hours.contains(10.5)) {
        std::cout << "10:30 AM is during work hours\n";
    }

    // Calculate total hours
    std::cout << "Total work hours: "
              << work_hours.measure() << '\n';

    // Display the set
    std::cout << "Work hours: " << work_hours << '\n';

    return 0;
}
```

### Compile and Run

```bash
# GCC
g++ -std=c++17 -I./include example.cpp -o example
./example

# Clang
clang++ -std=c++17 -I./include example.cpp -o example
./example

# MSVC
cl /std:c++17 /I.\include example.cpp
example.exe
```

**Output:**
```
10:30 AM is during work hours
Total work hours: 6
Work hours: [9,12] ∪ [14,17]
```

## Core Concepts

### Intervals

An **interval** represents a contiguous range of values with configurable boundaries:

```cpp
// Closed interval: includes both endpoints
auto closed = real_interval::closed(0, 10);        // [0, 10]

// Open interval: excludes both endpoints
auto open = real_interval::open(0, 10);           // (0, 10)

// Half-open intervals
auto left_open = real_interval::left_open(0, 10);  // (0, 10]
auto right_open = real_interval::right_open(0, 10); // [0, 10)

// Single point
auto point = real_interval::point(5);              // {5}

// Unbounded intervals (requires floating-point type)
auto positive = real_interval::greater_than(0);    // (0, ∞)
auto at_least_zero = real_interval::at_least(0);   // [0, ∞)
```

### Disjoint Interval Sets

A **disjoint interval set** maintains a collection of non-overlapping intervals:

```cpp
// Create empty set
real_set s;

// Add intervals (automatically merged if overlapping)
s.add(0, 10);     // [0, 10]
s.add(20, 30);    // [0, 10] ∪ [20, 30]
s.add(8, 22);     // [0, 30]  (merged all three!)

// Check membership
bool contains_15 = s.contains(15);  // true

// Get number of disjoint components
size_t count = s.size();  // 1 (all merged)
```

### Set Operations

DIS provides full Boolean algebra operations:

```cpp
auto a = real_set::from_string("[0,10] ∪ [20,30]");
auto b = real_set::from_string("[5,15] ∪ [25,35]");

// Union
auto union_ab = a | b;              // [0,15] ∪ [20,35]

// Intersection
auto intersect_ab = a & b;          // [5,10] ∪ [25,30]

// Complement
auto not_a = ~a;                    // (-∞,0) ∪ (10,20) ∪ (30,∞)

// Difference
auto diff = a - b;                  // [0,5) ∪ (10,20) ∪ (30,35]

// Symmetric difference
auto sym_diff = a ^ b;              // [0,5) ∪ (10,15] ∪ [20,25) ∪ (30,35]
```

## Common Patterns

### Pattern 1: Building Sets Incrementally

```cpp
real_set schedule;

// Add time slots
schedule.add(9, 10);     // 9-10 AM
schedule.add(11, 12);    // 11-12 PM
schedule.add(14, 16);    // 2-4 PM

// Add another slot (automatically merges if adjacent)
schedule.add(10, 11);    // Now [9,12] ∪ [14,16]
```

### Pattern 2: Parsing from Strings

```cpp
// Parse mathematical notation
auto meetings = real_set::from_string("[10,11] ∪ [14,15]");

// Alternative notations
auto same = real_set::from_string("[10,11] U [14,15]");  // ASCII 'U'
auto also = real_set::from_string("[10,11], [14,15]");   // Comma-separated
```

### Pattern 3: Filtering Intervals

```cpp
auto set = real_set::from_string("[1,2] ∪ [5,20] ∪ [25,30]");

// Keep only intervals longer than 10
auto large = set.filter([](const auto& i) {
    return i.length() > 10;
});
// Result: [5,20]
```

### Pattern 4: Analyzing Sets

```cpp
auto set = real_set::from_string("[0,10] ∪ [20,30] ∪ [40,50]");

// Get span (smallest interval containing all)
auto span = set.span();           // [0,50]

// Get gaps between intervals
auto gaps = set.gaps();           // [10,20] ∪ [30,40]

// Calculate density
double density = set.density();   // 0.6 (30/50)

// Total measure (length)
double total = set.measure();     // 30
```

## Next Steps

Now that you understand the basics, explore more advanced topics:

- **[User Guide](guide/basic-operations.md)**: Comprehensive tutorials
- **[API Reference](api/interval.md)**: Detailed API documentation
- **[Examples](examples.md)**: Real-world use cases
- **[Technical Report](technical-report/introduction.md)**: In-depth theory and implementation

## Quick Reference

### Type Aliases

```cpp
using namespace dis;

// Interval types
real_interval      // interval<double>
integer_interval   // interval<int>

// Set types
real_set          // disjoint_interval_set<real_interval>
integer_set       // disjoint_interval_set<integer_interval>

// Multi-dimensional
template<typename T> using rectangle = interval_nd<T, 2>;
template<typename T> using box = interval_nd<T, 3>;
```

### Common Operations

| Operation | Operator | Method |
|-----------|----------|--------|
| Union | `a \| b` | `a.unite(b)` |
| Intersection | `a & b` | `a.intersect(b)` |
| Complement | `~a` | `a.complement()` |
| Difference | `a - b` | `a.difference(b)` |
| Symmetric Diff | `a ^ b` | `a.symmetric_difference(b)` |

### Factory Methods

```cpp
// Bounded intervals
interval::closed(a, b)         // [a, b]
interval::open(a, b)          // (a, b)
interval::left_open(a, b)     // (a, b]
interval::right_open(a, b)    // [a, b)
interval::point(x)            // {x}

// Unbounded intervals
interval::unbounded()          // (-∞, ∞)
interval::at_least(a)         // [a, ∞)
interval::greater_than(a)     // (a, ∞)
interval::at_most(b)          // (-∞, b]
interval::less_than(b)        // (-∞, b)
```

## Troubleshooting

### Compilation Errors

**Issue**: `error: 'optional' is not a member of 'std'`

**Solution**: Ensure you're compiling with C++17 or later:
```bash
g++ -std=c++17 ...
```

**Issue**: `error: no matching function for call to 'interval::unbounded()'`

**Solution**: Unbounded intervals require floating-point types with infinity support:
```cpp
real_interval::unbounded();   // OK (double has infinity)
integer_interval::unbounded(); // ERROR (int doesn't have infinity)
```

### Runtime Issues

**Issue**: Intervals not merging as expected

**Check**: Boundary types must match for merging:
```cpp
auto a = real_interval::closed(0, 10);      // [0, 10]
auto b = real_interval::open(10, 20);       // (10, 20)
// These are NOT adjacent (gap at 10), won't merge

auto c = real_interval::right_open(0, 10);  // [0, 10)
auto d = real_interval::closed(10, 20);     // [10, 20]
// These ARE adjacent at 10, will merge to [0, 20]
```

## Getting Help

- **Documentation**: [Full API reference](api/interval.md)
- **Examples**: [Complete examples](examples.md)
- **Issues**: Report bugs on GitHub
- **Discussions**: Ask questions in GitHub Discussions

Happy interval computing!
