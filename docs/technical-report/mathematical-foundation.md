# Mathematical Foundation

This section establishes the mathematical theory underlying the Disjoint Interval Set library. We define intervals formally, establish their Boolean algebra structure, and analyze the complexity properties of our canonical representation.

## Intervals

### Definition

An **interval** over a totally ordered set \((T, \leq)\) is defined by its endpoints and boundary types:

\[I = [a, b] \cup \{(\cdot,\cdot), [\cdot,\cdot), (\cdot,\cdot], [\cdot,\cdot]\}\]

where \(a \leq b\) are the lower and upper bounds, and boundary indicators specify whether endpoints are included (closed) or excluded (open).

### Boundary Types

The four boundary configurations are:

- **Closed**: \([a, b]\) includes both endpoints
- **Open**: \((a, b)\) excludes both endpoints
- **Left-open**: \((a, b]\) excludes left, includes right
- **Right-open**: \([a, b)\) includes left, excludes right

### Membership

Point membership is defined as:

\[x \in [a,b] \iff a \leq x \leq b\]

\[x \in (a,b) \iff a < x < b\]

\[x \in (a,b] \iff a < x \leq b\]

\[x \in [a,b) \iff a \leq x < b\]

### Empty Intervals

An interval is **empty** if:

1. \(a > b\), or
2. \(a = b\) and at least one boundary is open

Empty intervals are represented canonically as \((0, -1)\) with both boundaries open, regardless of how they were constructed. This ensures unique representation.

### Special Intervals

For types supporting infinity (e.g., `double`):

- **Unbounded**: \((-\infty, \infty)\) contains all values
- **Half-bounded**: \([a, \infty)\), \((a, \infty)\), \((-\infty, b]\), \((-\infty, b)\)
- **Singleton**: \([a, a]\) contains exactly one point

---

## Disjoint Interval Sets

### Definition

A **disjoint interval set** is a finite collection of pairwise disjoint intervals:

\[D = \{I_1, I_2, \ldots, I_n\} \quad \text{where} \quad I_i \cap I_j = \emptyset \text{ for } i \neq j\]

### Canonical Form

We maintain intervals in **sorted order** by lower bounds:

\[I_1 < I_2 < \cdots < I_n\]

where \(I < J\) means the upper bound of \(I\) is less than the lower bound of \(J\), accounting for boundary types.

### Adjacency

Two intervals are **adjacent** if they share a boundary point with opposite closure properties:

- \((a,b]\) and \((b,c)\) are adjacent at \(b\)
- \([a,b)\) and \([b,c]\) are adjacent at \(b\)

Adjacent intervals can be merged into a single interval:

- \((a,b] \cup (b,c) = (a,c)\)
- \([a,b) \cup [b,c] = [a,c]\)

The canonical form merges all adjacent intervals, ensuring unique representation.

---

## Boolean Algebra Structure

### Definition

The set of all disjoint interval sets \(\mathcal{D}\) forms a **complete Boolean algebra**:

\[(\mathcal{D}, \cup, \cap, \neg, \emptyset, U)\]

where:

- **Join** (\(\vee\)): Set union \(\cup\)
- **Meet** (\(\wedge\)): Set intersection \(\cap\)
- **Complement** (\(\neg\)): Relative to universal set \(U = (-\infty, \infty)\)
- **Bottom**: Empty set \(\emptyset\)
- **Top**: Universal set \(U\)

### Axioms

All Boolean algebra axioms hold:

#### 1. Associativity
\[(A \cup B) \cup C = A \cup (B \cup C)\]
\[(A \cap B) \cap C = A \cap (B \cap C)\]

#### 2. Commutativity
\[A \cup B = B \cup A\]
\[A \cap B = B \cap A\]

#### 3. Distributivity
\[A \cap (B \cup C) = (A \cap B) \cup (A \cap C)\]
\[A \cup (B \cap C) = (A \cup B) \cap (A \cup C)\]

#### 4. Identity
\[A \cup \emptyset = A\]
\[A \cap U = A\]

#### 5. Complement
\[A \cup \neg A = U\]
\[A \cap \neg A = \emptyset\]

#### 6. Idempotence
\[A \cup A = A\]
\[A \cap A = A\]

#### 7. Absorption
\[A \cup (A \cap B) = A\]
\[A \cap (A \cup B) = A\]

#### 8. De Morgan's Laws
\[\neg(A \cup B) = \neg A \cap \neg B\]
\[\neg(A \cap B) = \neg A \cup \neg B\]

### Proof Sketch

These properties follow from:

1. The canonical form ensures unique representation
2. Operations are defined in terms of point-wise membership
3. Set-theoretic operations preserve the disjoint property
4. Normalization after each operation maintains sorted, merged form

A full formal verification would use proof assistants like Coq or Isabelle/HOL (future work).

---

## Measure Theory

### Interval Measure

For intervals over \(\mathbb{R}\), the **measure** (length) is:

\[\mu([a,b]) = \mu((a,b)) = \mu([a,b)) = \mu((a,b]) = b - a\]

Boundary types don't affect measure for real numbers (they have measure zero).

### Set Measure

For a disjoint interval set:

\[\mu(D) = \sum_{i=1}^{n} \mu(I_i)\]

This is well-defined because intervals are disjoint.

### Additional Metrics

**Span**: The smallest interval containing all intervals in the set:

\[\text{span}(D) = [\min_i a_i, \max_i b_i]\]

**Density**: Ratio of measure to span:

\[\text{density}(D) = \frac{\mu(D)}{\mu(\text{span}(D))}\]

**Gap Measure**: Total measure of gaps between intervals:

\[\text{gap\_measure}(D) = \mu(\text{span}(D)) - \mu(D)\]

---

## Computational Complexity

### Canonical Form Properties

Maintaining disjoint, sorted intervals provides:

1. **Unique Representation**: Each set has exactly one representation
2. **Efficient Queries**: Binary search for membership
3. **Linear Operations**: Zipper merge for set operations
4. **Space Efficiency**: Minimal interval count

### Complexity Analysis

Let \(n\) and \(m\) be the number of intervals in sets \(A\) and \(B\).

| Operation | Time | Space | Notes |
|-----------|------|-------|-------|
| **Construction** | \(O(n \log n)\) | \(O(n)\) | Sorting dominates |
| **Normalization** | \(O(n)\) | \(O(n)\) | Single pass merge |
| **Contains** | \(O(\log n)\) | \(O(1)\) | Binary search |
| **Union** | \(O(n + m)\) | \(O(n + m)\) | Linear merge |
| **Intersection** | \(O(nm)\) | \(O(\min(n,m))\) | Worst case; often better |
| **Complement** | \(O(n)\) | \(O(n + 1)\) | Invert boundaries |
| **Measure** | \(O(n)\) | \(O(1)\) | Sum lengths |

### Space-Time Tradeoffs

**Caching**: Measure could be cached for O(1) queries at cost of O(1) extra space and update overhead.

**Lazy Normalization**: Could defer merging until needed, trading space (more intervals) for time (avoid unnecessary merges).

**Index Structures**: For very large sets (10K+ intervals), interval trees provide O(log n + k) overlap queries where k is result size.

---

## Correctness Properties

### Invariants

The implementation maintains:

1. **Sorted**: Intervals ordered by lower bound
2. **Disjoint**: No overlapping intervals
3. **Non-adjacent**: Adjacent intervals are merged
4. **Non-empty**: Empty intervals are removed

These invariants are:

- **Established**: By normalization after construction
- **Maintained**: By all operations
- **Verified**: Through comprehensive testing (97.46% coverage)

### Type Safety

The type system ensures:

- Only totally ordered types can be interval boundaries (`Boundary` concept)
- Operations preserve interval types
- Compile-time intervals have zero runtime overhead
- Invalid constructions (e.g., reversed bounds) result in empty intervals

---

## Summary

The mathematical foundation of DIS provides:

- **Formal rigor**: Well-defined semantics based on set theory
- **Completeness**: Full Boolean algebra with all axioms satisfied
- **Efficiency**: Canonical form enables optimal algorithms
- **Correctness**: Strong invariants with comprehensive testing

This foundation enables both theoretical reasoning about program correctness and practical efficient implementation.
