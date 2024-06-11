# Disjoint Interval Set

The Disjoint Interval Set (DIS) equipped with a few operations
satisfies the concept of a Boolean algebra over sets of disjoint
intervals equipped with all the standard set-theoretic operations,
like intersection (*), union (+), and complement (~).

## Concept: Boolean Algebra

A Boolean algebra provides a powerful conceptual and mathematical framework.
It is a set of elements equipped with a few operations that satisfy
a few axioms. The operations are usually called union (+), intersection (*),
and complement (~). The axioms are usually called the Boolean laws.

## Constructors

The DIS supports the following constructors:

- **Empty Constructor**: `disjoint_interval_set()`

  Create an empty DIS.

- **Constructor From Iterable**: `disjoint_interval_set(intervals)`

  Create a DIS from an iterable of intervals.

- **Copy Constructor**: `disjoint_interval_set(disjoint_interval_set)`

  Create a copy of a DIS.

## Set-Theoretic Operations

The DIS supports the following set-theoretic operations:

- **Union**: `operator+(disjoint_interval_set, disjoint_interval_set)`

  Create a DIS that is the union of two DIS.

- **Intersection**: `operator*(disjoint_interval_set, disjoint_interval_set)`

  Create a DIS that is the intersection of two DIS.

- **Complement**: `operator~(disjoint_interval_set)`

  Create a DIS that is the complement of a DIS.

- **Set-Difference**: `operator-(disjoint_interval_set, disjoint_interval_set)`

  Create a DIS that is the set difference of two DIS.

- **Symmetric Difference**: `operator^(disjoint_interval_set, disjoint_interval_set)`

  Create a DIS that is the symmetric difference of two DIS.

## Predicates

The DIS supports the following predicates:

- **Empty**: `is_empty(disjoint_interval_set)`: Check if a DIS is empty.

- **Relational Predicates**: `==`, `!=`, `<`, `<=`, `>`, `>=`:

  Compare two DIS for equality, inequality, subset, proper subset, superset,
  and proper superset.

  These also work with intervals and values. For example, `DIS == interval`,
  since an interval can be considered a DIS with a single interval and a
  value can be considered an interval with a single value, `[value, value]`.

- **Set Membership**: `contains(disjoint_interval_set, value)`

  Check if a DIS contains a value.

## Interval Type

The DIS is parameterized by the interval type. The interval type must
satisfy the concept of an interval.

### Interval Concept

An interval is a pair of values that satisfy the following axioms:

- `infimum(interval)`: Return the infimum of the interval.
- `supremum(interval)`: Return the supremum of the interval.
- `contains(interval, value)`: Check if the interval contains a value.