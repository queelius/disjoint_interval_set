# Parser & Formatter API

String parsing and formatting for interval sets.

## Parser

### `from_string(std::string_view str)`

Parses mathematical notation:

```cpp
auto set = real_set::from_string("[0,5) ∪ (10,20] ∪ {25}");
```

**Supported Syntax:**

- Intervals: `[a,b]`, `(a,b)`, `[a,b)`, `(a,b]`
- Singletons: `{x}`
- Union: `∪`, `U`, `,` (comma)
- Infinity: `∞`, `-∞`

## Formatter

### `to_string()`

Converts set to string representation:

```cpp
std::string notation = set.to_string();
```

### Stream Output

```cpp
std::cout << set << '\n';  // Uses default formatting
```
