# Type Aliases

Common type aliases for convenience.

## Interval Types

```cpp
namespace dis {
    using real_interval = interval<double>;
    using integer_interval = interval<int>;
}
```

## Set Types

```cpp
namespace dis {
    using real_set = disjoint_interval_set<real_interval>;
    using integer_set = disjoint_interval_set<integer_interval>;
}
```
