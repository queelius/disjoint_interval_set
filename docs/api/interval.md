# Interval API Reference

Complete API documentation for the `interval<T>` class template.

## Class Template

```cpp
template<Boundary T>
class interval;
```

### Template Parameters

- **T**: The value type for interval boundaries. Must satisfy the `Boundary` concept (totally ordered and regular).

### Common Type Aliases

```cpp
namespace dis {
    using real_interval = interval<double>;
    using integer_interval = interval<int>;
}
```

## Named Constructors (Factory Methods)

### Bounded Intervals

#### `closed(T lower, T upper)`
```cpp
[[nodiscard]] static constexpr interval closed(T lower, T upper) noexcept;
```
Creates a closed interval \([a, b]\) that includes both endpoints.

**Example:**
```cpp
auto i = real_interval::closed(0, 10);  // [0, 10]
assert(i.contains(0));   // true
assert(i.contains(10));  // true
```

#### `open(T lower, T upper)`
```cpp
[[nodiscard]] static constexpr interval open(T lower, T upper) noexcept;
```
Creates an open interval \((a, b)\) that excludes both endpoints.

**Example:**
```cpp
auto i = real_interval::open(0, 10);  // (0, 10)
assert(!i.contains(0));   // false
assert(!i.contains(10));  // false
assert(i.contains(5));    // true
```

#### `left_open(T lower, T upper)`
```cpp
[[nodiscard]] static constexpr interval left_open(T lower, T upper) noexcept;
```
Creates a left-open interval \((a, b]\).

#### `right_open(T lower, T upper)`
```cpp
[[nodiscard]] static constexpr interval right_open(T lower, T upper) noexcept;
```
Creates a right-open interval \([a, b)\).

### Special Intervals

#### `point(T value)`
```cpp
[[nodiscard]] static constexpr interval point(T value) noexcept;
```
Creates a singleton interval \(\{x\}\) containing exactly one point.

#### `empty()`
```cpp
[[nodiscard]] static constexpr interval empty() noexcept;
```
Creates an empty interval \(\emptyset\).

### Unbounded Intervals

Requires `std::numeric_limits<T>::has_infinity`.

#### `unbounded()`
```cpp
[[nodiscard]] static constexpr interval unbounded() noexcept;
```
Creates \((-\infty, \infty)\).

#### `at_least(T lower)`
```cpp
[[nodiscard]] static constexpr interval at_least(T lower) noexcept;
```
Creates \([a, \infty)\).

#### `greater_than(T lower)`
```cpp
[[nodiscard]] static constexpr interval greater_than(T lower) noexcept;
```
Creates \((a, \infty)\).

#### `at_most(T upper)`
```cpp
[[nodiscard]] static constexpr interval at_most(T upper) noexcept;
```
Creates \((-\infty, b]\).

#### `less_than(T upper)`
```cpp
[[nodiscard]] static constexpr interval less_than(T upper) noexcept;
```
Creates \((-\infty, b)\).

## Member Functions

### Queries

#### `is_empty()`
```cpp
constexpr bool is_empty() const noexcept;
```
Returns `true` if interval contains no points.

#### `contains(T value)`
```cpp
constexpr bool contains(T value) const;
```
Tests if value is in interval.

#### `overlaps(const interval& other)`
```cpp
constexpr bool overlaps(const interval& other) const;
```
Tests if intervals have non-empty intersection.

### Accessors

#### `lower_bound()`, `upper_bound()`
```cpp
constexpr std::optional<T> lower_bound() const noexcept;
constexpr std::optional<T> upper_bound() const noexcept;
```
Returns bounds, or `std::nullopt` for empty intervals.

### Measures

(Requires `std::is_arithmetic_v<T>`)

#### `length()`
```cpp
constexpr T length() const;
```
Returns \(b - a\).

#### `midpoint()`
```cpp
constexpr T midpoint() const;
```
Returns \((a + b) / 2\).

### Operations

#### `intersect(const interval& other)`
```cpp
constexpr interval intersect(const interval& other) const;
```
Returns intersection.

#### `hull(const interval& other)`
```cpp
constexpr std::optional<interval> hull(const interval& other) const;
```
Returns convex hull if intervals overlap or are adjacent, otherwise `std::nullopt`.

## Operators

```cpp
// Intersection
friend interval operator&(const interval& a, const interval& b);

// Comparison
friend auto operator<=>(const interval& a, const interval& b);
friend bool operator==(const interval& a, const interval& b);

// Stream output
friend std::ostream& operator<<(std::ostream& os, const interval& i);
```

## Complete Example

See [Getting Started](../getting-started.md) and [Examples](../examples.md) for usage examples.
