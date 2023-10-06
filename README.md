Disjoint Interval Set
=====================

The Disjoint Interval Set (DIS) equipped with a few operations
satisfies the concept of a Boolean algebra over sets of disjoint
intervals equipped with all the standard set-theoretic operations,
like intersection (*), union (+), and complement (~).

### Boolean Algebra
A Boolean algebra provides a powerful conceptual and mathematical framework.
It is a set of elements equipped with a few operations that satisfy
a few axioms. The operations are usually called union (+), intersection (*),
and complement (~). The axioms are usually called the Boolean laws.

## DIS Operations
The DIS supports the following operations:

1. `disjoint_interval_set()`: Create an empty DIS.
2. `disjoint_interval_set(intervals)`: Create a DIS from an iterable of intervals.
3. `disjoint_interval_set(disjoint_interval_set)`: Create a copy of a DIS.
4. `operator+(disjoint_interval_set, disjoint_interval_set)`: Union of two DIS.
5. `operator*(disjoint_interval_set, disjoint_interval_set)`: Intersection of two DIS.
6. `operator~(disjoint_interval_set)`: Complement of a DIS.

## DIS Predicates
The DIS supports the following predicates:

1. `is_empty(disjoint_interval_set)`: Check if a DIS is empty.

2. relational predicates `==`, `!=`, `<`, `<=`, `>`, `>=`:

   Compare two DIS for equality, inequality, subset, proper subset, superset,
   and proper superset.

   These also work with intervals and values. For example, `DIS == interval`,
   since an interval can be considered a DIS with a single interval and a
   value can be considered an interval with a single value, `[value, value]`.

3. `contains(disjoint_interval_set, value)`: Check if a DIS contains a value.


## Generic Programming

The DIS is parameterized by the interval type. The interval type must
satisfy the concept of an interval:


### Interval Concept
An interval is a pair of values that satisfy the following axioms:

1. `lower(interval)`: Return the lower bound of the interval.
2. `upper(interval)`: Return the upper bound of the interval.
3. `contains(interval, value)`: Check if the interval contains a value. 