# Disjoint Interval Set API Reference

Complete API documentation for the `disjoint_interval_set<I>` class template.

## Class Template

```cpp
template<typename I>
class disjoint_interval_set;
```

### Template Parameters

- **I**: The interval type. Typically `interval<T>`.

### Common Type Aliases

```cpp
namespace dis {
    using real_set = disjoint_interval_set<real_interval>;
    using integer_set = disjoint_interval_set<integer_interval>;
}
```

## Constructors

### Default Constructor
```cpp
disjoint_interval_set() = default;
```

### From Interval
```cpp
explicit disjoint_interval_set(I interval);
```

### From Initializer List
```cpp
disjoint_interval_set(std::initializer_list<I> intervals);
```

### From Range
```cpp
template<std::ranges::input_range R>
explicit disjoint_interval_set(R&& range);
```

## Named Constructors

### `empty()`
```cpp
[[nodiscard]] static disjoint_interval_set empty();
```

### `from_string(std::string_view str)`
```cpp
[[nodiscard]] static disjoint_interval_set from_string(std::string_view str);
```
Parses mathematical notation like `"[0,5) ∪ (10,20]"`.

## Core Operations

### Queries

#### `is_empty()`
```cpp
[[nodiscard]] bool is_empty() const noexcept;
```

#### `size()`
```cpp
[[nodiscard]] size_type size() const noexcept;
```
Returns number of disjoint intervals.

#### `contains(value_type value)`
```cpp
[[nodiscard]] bool contains(value_type value) const;
```
Tests membership. O(log n) via binary search.

#### `overlaps(const disjoint_interval_set& other)`
```cpp
[[nodiscard]] bool overlaps(const disjoint_interval_set& other) const;
```

### Set Operations

All operations return a new set (value semantics).

#### Union
```cpp
disjoint_interval_set unite(const disjoint_interval_set& other) const;
friend disjoint_interval_set operator|(const disjoint_interval_set& a,
                                       const disjoint_interval_set& b);
```

#### Intersection
```cpp
disjoint_interval_set intersect(const disjoint_interval_set& other) const;
friend disjoint_interval_set operator&(const disjoint_interval_set& a,
                                       const disjoint_interval_set& b);
```

#### Complement
```cpp
disjoint_interval_set complement() const;
friend disjoint_interval_set operator~(const disjoint_interval_set& a);
```

#### Difference
```cpp
disjoint_interval_set difference(const disjoint_interval_set& other) const;
friend disjoint_interval_set operator-(const disjoint_interval_set& a,
                                       const disjoint_interval_set& b);
```

#### Symmetric Difference
```cpp
disjoint_interval_set symmetric_difference(const disjoint_interval_set& other) const;
friend disjoint_interval_set operator^(const disjoint_interval_set& a,
                                       const disjoint_interval_set& b);
```

### Fluent Interface

#### `add(I interval)` / `add(T lower, T upper)`
```cpp
disjoint_interval_set& add(I interval) &;
disjoint_interval_set&& add(I interval) &&;
disjoint_interval_set& add(value_type lower, value_type upper) &;
```

#### `remove(I interval)` / `erase(I interval)`
```cpp
disjoint_interval_set& remove(I interval) &;
disjoint_interval_set& erase(I interval) &;
```

### Analysis Operations

#### `span()`
```cpp
[[nodiscard]] I span() const;
```
Returns smallest interval containing all intervals.

#### `gaps()`
```cpp
[[nodiscard]] disjoint_interval_set gaps() const;
```
Returns intervals between components.

#### `measure()`
```cpp
[[nodiscard]] value_type measure() const;
```
Returns total length.

#### `density()`
```cpp
[[nodiscard]] double density() const;
```
Returns `measure() / span().length()`.

### Functional Operations

#### `filter(Predicate pred)`
```cpp
template<typename Predicate>
disjoint_interval_set filter(Predicate pred) const;
```

#### `for_each(Function fn)`
```cpp
template<typename Function>
void for_each(Function fn) const;
```

## Iteration

```cpp
const_iterator begin() const noexcept;
const_iterator end() const noexcept;
```

Use in range-based for loops:
```cpp
for (const auto& interval : my_set) {
    // Process interval
}
```

## Complete Example

See [Examples](../examples.md) for comprehensive usage examples.
