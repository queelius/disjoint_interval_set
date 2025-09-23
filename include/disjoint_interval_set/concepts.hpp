#pragma once

#include <concepts>
#include <optional>
#include <type_traits>

namespace disjoint_interval_set {

/**
 * Core concepts that define the interval algebra abstraction.
 * These concepts establish the mathematical contract for interval types.
 */

/**
 * @brief Concept for types that can be used as interval boundaries.
 *
 * Requires total ordering and equality comparison.
 */
template<typename T>
concept BoundaryType = std::totally_ordered<T> && std::copyable<T>;

/**
 * @brief Concept for interval types that model mathematical intervals.
 *
 * An interval type must provide:
 * - A value_type member type
 * - Emptiness testing
 * - Membership testing
 * - Boundary access (infimum/supremum)
 */
template<typename I>
concept IntervalType = requires(const I& interval, typename I::value_type value) {
    typename I::value_type;
    requires BoundaryType<typename I::value_type>;

    // Core predicates
    { interval.is_empty() } -> std::convertible_to<bool>;
    { interval.contains(value) } -> std::convertible_to<bool>;

    // Boundary access - these should be provided as friend functions
    { interval.lower() } -> std::convertible_to<std::optional<typename I::value_type>>;
    { interval.upper() } -> std::convertible_to<std::optional<typename I::value_type>>;
};

/**
 * @brief Concept for types that support interval set operations.
 *
 * Extends IntervalType with algebraic operations.
 */
template<typename I>
concept IntervalAlgebra = IntervalType<I> && requires(const I& a, const I& b) {
    // Set operations
    { a * b } -> std::same_as<I>;  // intersection
    { adjacent(a, b) } -> std::convertible_to<bool>;
    { a < b } -> std::convertible_to<bool>;  // subset relation
    { a == b } -> std::convertible_to<bool>;
};

/**
 * @brief Concept for disjoint interval set types.
 *
 * A disjoint interval set must provide iteration over its intervals
 * and support set-theoretic operations.
 */
template<typename S>
concept DisjointIntervalSet = requires(const S& set, typename S::value_type value) {
    typename S::interval_type;
    typename S::value_type;
    typename S::const_iterator;

    requires IntervalType<typename S::interval_type>;

    // Container operations
    { set.empty() } -> std::convertible_to<bool>;
    { set.begin() } -> std::same_as<typename S::const_iterator>;
    { set.end() } -> std::same_as<typename S::const_iterator>;
    { set.contains(value) } -> std::convertible_to<bool>;

    // Boundary access
    { set.infimum() } -> std::convertible_to<std::optional<typename S::value_type>>;
    { set.supremum() } -> std::convertible_to<std::optional<typename S::value_type>>;
};

/**
 * @brief Concept for types that support disjoint interval set algebra.
 */
template<typename S>
concept DisjointIntervalSetAlgebra = DisjointIntervalSet<S> && requires(const S& a, const S& b) {
    // Boolean algebra operations
    { a + b } -> std::same_as<S>;  // union
    { a * b } -> std::same_as<S>;  // intersection
    { ~a } -> std::same_as<S>;     // complement
    { a - b } -> std::same_as<S>;  // difference
    { a ^ b } -> std::same_as<S>;  // symmetric difference

    // Relation predicates
    { a <= b } -> std::convertible_to<bool>;  // subset
    { a >= b } -> std::convertible_to<bool>;  // superset
    { a == b } -> std::convertible_to<bool>;  // equality
    { a != b } -> std::convertible_to<bool>;  // inequality
    { a < b } -> std::convertible_to<bool>;   // proper subset
    { a > b } -> std::convertible_to<bool>;   // proper superset
};

} // namespace disjoint_interval_set