# Introduction

Intervals are fundamental mathematical objects appearing throughout computer science: from computational geometry algorithms to scheduling systems, from numerical analysis to access control mechanisms. While several interval libraries exist—notably Boost.ICL (Interval Container Library)—most treat intervals as containers requiring explicit management of overlaps and adjacency. This approach places burden on the programmer and obscures the underlying mathematical structure.

We present a different philosophy: **treating interval sets as first-class mathematical objects that form a complete Boolean algebra**. This perspective leads naturally to intuitive operations, compositional design, and strong correctness guarantees. Our key insight is that by maintaining a canonical form (disjoint, sorted intervals), we can provide both mathematical elegance and computational efficiency.

## Motivation

Consider a simple scheduling problem: finding free time in a calendar. Traditional approaches require manual loop over intervals, explicit overlap checking, and careful boundary handling. With DIS, the solution is natural:

```cpp
auto availability = real_set{}.add(9, 17);  // 9 AM - 5 PM
auto meetings = real_set::from_string("[10,11] ∪ [14,15]");
auto free_time = availability - meetings;   // Set difference
```

This code:
- **Reads like mathematics**: The operations mirror set-theoretic notation
- **Handles all edge cases**: Boundaries, overlaps, adjacency handled automatically
- **Provides guarantees**: Result is always in canonical form
- **Is efficient**: O(n + m) complexity for n and m intervals

## Contributions

This technical report presents:

1. **Complete Boolean Algebra Implementation**
   Full support for set-theoretic operations (union, intersection, complement, difference, symmetric difference) satisfying all Boolean algebra axioms with formal correctness properties.

2. **Elegant API Design**
   Multiple equivalent notations (operators, named methods, fluent interface) enabling natural expression for different contexts and coding styles.

3. **Compile-Time Features**
   Zero-overhead interval validation and computation using template metaprogramming. Static intervals enable bounded types with compile-time checks.

4. **Domain-Specific Language**
   Parser for standard mathematical notation supporting round-trip serialization: `"[0,5) ∪ (10,20] ∪ {25}"`.

5. **Multi-Dimensional Extension**
   Natural generalization to N-dimensional hyperrectangles for spatial and spatio-temporal applications.

6. **Rich Query Interface**
   Comprehensive operations including `gaps()`, `span()`, `density()`, and `measure()` for interval analysis.

## Comparison with Boost.ICL

Boost.ICL provides comprehensive interval container functionality but adopts a container-centric design philosophy. Our library differs fundamentally:

| Aspect | DIS | Boost.ICL |
|--------|-----|-----------|
| **Philosophy** | Mathematical Boolean algebra | Container semantics |
| **API Style** | Natural operators (`\|`, `&`, `~`) | Method calls |
| **String Parsing** | Built-in DSL | Not available |
| **Dependencies** | Zero | Requires Boost |
| **Compile-Time** | First-class support | Limited |
| **Multi-Dimensional** | Native support | Requires extension |
| **Learning Curve** | Intuitive for mathematical users | Steeper |

## Design Goals

The library is designed around three core principles:

### 1. Mathematical Rigor
All operations maintain Boolean algebra properties. The implementation satisfies associativity, commutativity, distributivity, De Morgan's laws, and complement properties. This mathematical foundation provides strong correctness guarantees.

### 2. Ergonomic API
We provide three equivalent ways to express operations:

```cpp
// Symbolic operators (concise)
auto result = (a | b) & ~c;

// Named methods (self-documenting)
auto result = a.unite(b).intersect(c.complement());

// Fluent interface (chainable)
auto result = real_set{}
    .add(interval1)
    .add(interval2)
    .remove(interval3);
```

Each style is optimal for different contexts, respecting various coding preferences.

### 3. Performance
Despite the mathematical abstraction, DIS achieves excellent performance:

- **O(log n)** membership queries via binary search
- **O(n + m)** set operations via linear merge
- **Zero runtime overhead** for compile-time intervals
- **Automatic normalization** maintains canonical form efficiently

## Structure of This Report

The remainder of this report is organized as follows:

- **[Mathematical Foundation](mathematical-foundation.md)**: Formal definitions, Boolean algebra properties, and complexity analysis
- **[Design & Implementation](design-implementation.md)**: API design philosophy, core algorithms, and implementation techniques
- **[Performance Analysis](performance.md)**: Complexity analysis, benchmarks, and optimization strategies
- **[Applications](applications.md)**: Real-world use cases across multiple domains
- **[Related Work](related-work.md)**: Comparison with existing interval libraries and theoretical foundations

## Implementation Status

The library is production-ready with:

- **97.46%** test coverage on core implementation
- **90.32%** overall test coverage
- **94 test cases** covering edge cases and real-world scenarios
- **Zero dependencies** beyond C++17 standard library
- **Header-only** for easy integration

The complete source code is available under the MIT license.
