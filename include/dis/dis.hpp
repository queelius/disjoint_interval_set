#pragma once

/**
 * @brief Disjoint Interval Set Library
 *
 * A elegant, composable library for working with disjoint interval sets,
 * implementing a Boolean algebra with set-theoretic operations.
 *
 * Core principles:
 * - Mathematical rigor: Models true mathematical intervals
 * - Composability: All operations compose naturally
 * - Simplicity: Clean API that's a joy to use
 * - Performance: Zero-cost abstractions where possible
 * - Flexibility: Multiple ways to express the same concept
 *
 * Example usage:
 * @code
 * using namespace dis;
 *
 * // Create intervals with clear factory methods
 * auto i1 = real_interval::closed(0, 10);
 * auto i2 = real_interval::open(5, 15);
 * auto i3 = real_interval::point(7.5);
 *
 * // Build sets with fluent interface
 * auto set = real_set{}
 *     .add(0, 10)
 *     .add(20, 30)
 *     .add(25, 35);  // Automatically merged with [20, 30]
 *
 * // Parse from mathematical notation
 * auto parsed = real_set::from_string("[0,10) ∪ (20,30] ∪ {50}");
 *
 * // Rich query interface
 * auto gaps = set.gaps();           // Intervals between components
 * auto span = set.span();           // Smallest interval containing all
 * auto density = set.density();     // Measure / span.length
 * auto measure = set.measure();     // Total length
 *
 * // Set operations with multiple notations
 * auto union_set = set1 | set2;           // Union
 * auto intersection = set1 & set2;        // Intersection
 * auto difference = set1 - set2;          // Difference
 * auto symmetric = set1 ^ set2;          // Symmetric difference
 * auto complement = ~set1;               // Complement
 *
 * // Functional operations
 * auto filtered = set.filter([](const auto& i) {
 *     return i.length() > 5;
 * });
 *
 * set.for_each([](const auto& interval) {
 *     std::cout << interval << '\n';
 * });
 * @endcode
 */

// Core components
#include "core/interval.hpp"
#include "core/disjoint_interval_set.hpp"

// I/O utilities
#include "io/parser.hpp"
#include "io/format.hpp"

// Bring core types into dis namespace
namespace dis {
    // Already defined in core headers
}

// Optional: Bring most common types to global namespace
// (User can choose to use these or fully qualify)
using dis::real_interval;
using dis::integer_interval;
using dis::real_set;
using dis::integer_set;