# Design & Implementation

This section describes the design philosophy and implementation techniques used in the DIS library. We emphasize API ergonomics, zero-cost abstractions, and compositional design.

## Core Abstraction: The Interval Class

### Interface Design

The `interval<T>` template provides the fundamental interval abstraction:

```cpp
template<Boundary T>
class interval {
public:
    using value_type = T;

    // === Named Constructors (Factory Methods) ===

    [[nodiscard]] static constexpr interval closed(T lower, T upper);
    [[nodiscard]] static constexpr interval open(T lower, T upper);
    [[nodiscard]] static constexpr interval left_open(T lower, T upper);
    [[nodiscard]] static constexpr interval right_open(T lower, T upper);
    [[nodiscard]] static constexpr interval point(T value);
    [[nodiscard]] static constexpr interval empty();

    // Unbounded intervals (requires infinity support)
    [[nodiscard]] static constexpr interval unbounded();
    [[nodiscard]] static constexpr interval at_least(T lower);
    [[nodiscard]] static constexpr interval at_most(T upper);
    [[nodiscard]] static constexpr interval greater_than(T lower);
    [[nodiscard]] static constexpr interval less_than(T upper);

    // === Core Operations ===

    constexpr bool contains(T value) const;
    constexpr bool overlaps(const interval& other) const;
    constexpr interval intersect(const interval& other) const;
    constexpr std::optional<interval> hull(const interval& other) const;

    // === Queries ===

    constexpr bool is_empty() const;
    constexpr std::optional<T> lower_bound() const;
    constexpr std::optional<T> upper_bound() const;
    constexpr T length() const requires std::is_arithmetic_v<T>;
    constexpr T midpoint() const requires std::is_arithmetic_v<T>;
};
```

### Design Rationale

**Named Constructors**: Instead of:

```cpp
interval(0, 10, true, false);  // What does this mean?
```

We provide:

```cpp
interval::right_open(0, 10);   // Clear intent!
```

This eliminates cognitive overhead and reduces errors.

**Const by Default**: All operations return new intervals rather than modifying in-place, supporting functional programming style and enabling thread safety.

**Constexpr Everything**: All operations are `constexpr`, enabling compile-time computation:

```cpp
constexpr auto i = real_interval::closed(0, 10);
static_assert(i.contains(5));  // Verified at compile-time
```

**Requires Clauses**: C++20 `requires` clauses provide clear compile-time constraints:

```cpp
constexpr T length() const requires std::is_arithmetic_v<T>;
```

This generates better error messages when misused.

---

## Disjoint Interval Set Class

### Core Implementation

```cpp
template<typename I>
class disjoint_interval_set {
    std::vector<I> intervals_;  // Sorted, disjoint, non-adjacent

    // Maintains canonical form
    void normalize() {
        if (intervals_.empty()) return;

        // Sort by lower bounds
        std::ranges::sort(intervals_, [](const I& a, const I& b) {
            return *a.lower_bound() < *b.lower_bound();
        });

        // Merge overlapping/adjacent intervals
        size_t write = 0;
        for (size_t read = 1; read < intervals_.size(); ++read) {
            if (auto merged = intervals_[write].hull(intervals_[read])) {
                intervals_[write] = *merged;
            } else {
                intervals_[++write] = intervals_[read];
            }
        }
        intervals_.resize(write + 1);
    }

public:
    // Multiple API styles...
};
```

### The Normalization Algorithm

The `normalize()` method is central to maintaining canonical form:

1. **Sort**: Order intervals by lower bound (O(n log n))
2. **Merge**: Single-pass merge of overlapping/adjacent intervals (O(n))

The key insight: `hull()` returns `std::optional<interval>`, succeeding only for overlapping or adjacent intervals. This elegantly handles both cases.

**Example:**

```cpp
// Input: [(5,10), [2,4], (3,6], [10,15]]
// After sort: [[2,4], (3,6], (5,10), [10,15]]
// After merge: [[2,10), [10,15]]  // Adjacent intervals merged
// Final: [[2,15]]                  // All intervals merged
```

---

## API Design Philosophy

### Multiple Equivalent Notations

We provide three ways to express the same operations:

#### 1. Mathematical Operators (Concise)

```cpp
auto result = (a | b) & ~c;  // (A ∪ B) ∩ ¬C
```

**When to use**: Complex expressions, symbolic manipulation, mathematical contexts.

#### 2. Named Methods (Explicit)

```cpp
auto result = a.unite(b).intersect(c.complement());
```

**When to use**: Self-documenting code, unfamiliar teams, API discovery.

#### 3. Fluent Interface (Chainable)

```cpp
auto result = real_set{}
    .add(0, 10)
    .add(20, 30)
    .remove(real_interval::closed(5, 25));
```

**When to use**: Building complex sets step-by-step, imperative style.

### Operator Overloading

We carefully chose operators to match mathematical convention:

| Operation | Operator | Method | Mathematical |
|-----------|----------|--------|--------------|
| Union | `a \| b` | `a.unite(b)` | \(A \cup B\) |
| Intersection | `a & b` | `a.intersect(b)` | \(A \cap B\) |
| Complement | `~a` | `a.complement()` | \(\neg A\) |
| Difference | `a - b` | `a.difference(b)` | \(A \setminus B\) |
| Symmetric Diff | `a ^ b` | `a.symmetric_difference(b)` | \(A \triangle B\) |

**Precedence**: C++ operator precedence matches mathematical convention for these operations.

---

## Compile-Time Intervals

### Static Interval Template

For compile-time validation:

```cpp
template<auto Min, auto Max, bool LeftClosed, bool RightClosed>
struct static_interval {
    static constexpr auto min = Min;
    static constexpr auto max = Max;
    static constexpr bool left_closed = LeftClosed;
    static constexpr bool right_closed = RightClosed;

    static constexpr bool contains(auto value) {
        const bool left_ok = LeftClosed ? (value >= min) : (value > min);
        const bool right_ok = RightClosed ? (value <= max) : (value < max);
        return left_ok && right_ok;
    }

    static constexpr auto length() { return max - min; }
};
```

### Application: Bounded Types

```cpp
template<int Min, int Max>
class percentage {
    using bounds = static_interval<Min, Max, true, true>;
    int value_;

public:
    constexpr percentage(int v) : value_(v) {
        if (!bounds::contains(v))
            throw std::out_of_range("Value out of bounds");
    }

    constexpr operator int() const { return value_; }
};

using percent = percentage<0, 100>;

// Compile-time verification
static_assert(percent::bounds::contains(50));
static_assert(!percent::bounds::contains(150));

// Runtime validation
percent p1(75);   // OK
percent p2(150);  // Throws exception
```

This achieves **zero runtime overhead** for valid inputs in optimized builds.

---

## Domain-Specific Language

### Parser Design

The parser supports standard mathematical notation:

```cpp
// Supported syntax:
"[0,10]"              // Closed interval
"(0,10)"              // Open interval
"[0,10)"              // Right-open interval
"{5}"                 // Singleton
"[0,5) ∪ (10,20]"     // Union with Unicode
"[0,5) U (10,20]"     // Union with ASCII
"[0,5), (10,20]"      // Comma-separated union
"[0,∞)"               // Unbounded
"(-∞,0]"              // Unbounded
```

### Implementation

Recursive descent parser with these production rules:

```
expression := term (union_op term)*
term       := interval | singleton | empty
interval   := '[' number ',' number ']'
            | '(' number ',' number ')'
            | '[' number ',' number ')'
            | '(' number ',' number ']'
singleton  := '{' number '}'
empty      := '{' '}'
union_op   := '∪' | 'U' | ','
number     := digit+ ('.' digit+)? | '∞' | '-∞'
```

**Round-Trip Property**: `from_string(to_string(s)) == s` for all valid sets.

---

## Multi-Dimensional Extension

### Hyperrectangles

Intervals generalize naturally to N dimensions:

```cpp
template<typename T, size_t N>
class interval_nd {
    std::array<interval<T>, N> axes_;

public:
    bool contains(const std::array<T, N>& point) const {
        for (size_t i = 0; i < N; ++i) {
            if (!axes_[i].contains(point[i]))
                return false;
        }
        return true;
    }

    T measure() const {
        T result = 1;
        for (const auto& axis : axes_) {
            result *= axis.length();
        }
        return result;
    }

    interval_nd intersect(const interval_nd& other) const {
        interval_nd result;
        for (size_t i = 0; i < N; ++i) {
            result.axes_[i] = axes_[i].intersect(other.axes_[i]);
        }
        return result;
    }
};

// Convenient aliases
template<typename T> using rectangle = interval_nd<T, 2>;
template<typename T> using box = interval_nd<T, 3>;
```

### Applications

**2D Spatial Queries:**

```cpp
auto screen = rectangle<int>::closed(0, 1920, 0, 1080);
auto window = rectangle<int>::closed(100, 500, 100, 400);

assert(screen.contains(window));
assert(window.area() == 120000);
```

**3D Collision Detection:**

```cpp
auto room = box<double>::closed(0, 10, 0, 10, 0, 3);
auto furniture = box<double>::closed(2, 4, 3, 5, 0, 1);

bool fits = room.contains(furniture);
double volume = furniture.volume();
```

---

## Memory Layout and Optimization

### Interval Storage

Each `interval<T>` contains:

```cpp
T lower_;            // sizeof(T)
T upper_;            // sizeof(T)
bool left_closed_;   // 1 byte (could pack into padding)
bool right_closed_;  // 1 byte (could pack into padding)
```

For `real_interval` (T=double): 16 + 2 = 18 bytes, typically padded to 24 bytes.

### Set Storage

`disjoint_interval_set` uses `std::vector<I>`:

- **Small sets**: Direct storage, no heap allocation for capacity ≤ N (small vector optimization in some STL implementations)
- **Large sets**: Heap allocation, exponential growth strategy
- **Move semantics**: Efficient transfers without copying

### Optimization Techniques

1. **Reserve**: Pre-allocate capacity when size is known
   ```cpp
   real_set s;
   s.reserve(100);  // Avoid reallocation
   ```

2. **Move construction**: Avoid copies
   ```cpp
   auto s = real_set{}.add(1,2).add(3,4);  // No copies
   ```

3. **Binary search**: O(log n) contains queries
4. **Single-pass normalization**: O(n) after construction

---

## Summary

The DIS library demonstrates how careful API design can make complex mathematical concepts both accessible and efficient:

- **Multiple notations** respect different coding styles
- **Named constructors** improve readability
- **Compile-time features** provide zero-cost abstractions
- **Canonical form** enables efficient algorithms
- **Value semantics** support functional programming

The result is a library that is simultaneously mathematically rigorous, ergonomically pleasant, and computationally efficient.
