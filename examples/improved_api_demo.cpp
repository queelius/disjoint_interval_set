#include <iostream>
#include <cassert>
#include "../include/disjoint_interval_set/interval_v2.hpp"
#include "../include/disjoint_interval_set/disjoint_interval_set_v2.hpp"

using disjoint_interval_set::interval;
using disjoint_interval_set::make_interval_set;

void demonstrate_interval_api() {
    std::cout << "=== Interval API Improvements ===\n\n";

    // 1. Named factory methods for clarity
    auto closed = interval<double>::closed(1.0, 5.0);     // [1, 5]
    auto open = interval<double>::open(1.0, 5.0);         // (1, 5)
    auto left_open = interval<double>::left_open(1.0, 5.0);   // (1, 5]
    auto right_open = interval<double>::right_open(1.0, 5.0);  // [1, 5)
    auto singleton = interval<double>::singleton(3.14);        // [3.14, 3.14]
    auto empty = interval<double>::make_empty();                    // empty interval

    std::cout << "Factory methods create clear, self-documenting intervals\n";
    assert(closed.contains(1.0) && closed.contains(5.0));
    assert(!open.contains(1.0) && !open.contains(5.0));
    assert(singleton.is_singleton());
    assert(empty.is_empty());

    // 2. Builder pattern for complex construction
    auto complex_interval = interval<double>::make()
        .from(2.5)
        .to(7.8)
        .open_left()
        .closed_right()
        .build();  // (2.5, 7.8]

    std::cout << "Builder pattern allows fluent construction\n";
    assert(!complex_interval.contains(2.5));
    assert(complex_interval.contains(7.8));

    // 3. Named methods alongside operators
    auto i1 = interval<double>::closed(1, 5);
    auto i2 = interval<double>::closed(3, 7);

    auto intersection1 = i1.intersect(i2);  // Named method
    auto intersection2 = i1 * i2;           // Operator (mathematical notation)
    auto intersection3 = i1 & i2;           // Operator (set notation)

    assert(intersection1 == intersection2);
    assert(intersection2 == intersection3);

    std::cout << "Multiple ways to express operations for different preferences\n";

    // 4. Rich predicate methods
    assert(i1.overlaps(i2));
    assert(!i1.disjoint(i2));
    assert(!i1.subset_of(i2));
    assert(interval<double>::closed(2, 4).subset_of(i1));

    // 5. Boundary queries with clear semantics
    auto bounds = interval<double>::closed(10, 20);
    assert(bounds.lower().value() == 10);
    assert(bounds.upper().value() == 20);
    assert(bounds.is_left_closed());
    assert(bounds.is_right_closed());
    assert(bounds.is_bounded());

    std::cout << "Clear and consistent boundary access methods\n\n";
}

void demonstrate_interval_set_api() {
    std::cout << "=== Disjoint Interval Set API Improvements ===\n\n";

    // 1. Multiple construction methods
    disjoint_interval_set::disjoint_interval_set<interval<double>> set1;  // Empty set

    auto set2 = disjoint_interval_set::disjoint_interval_set<interval<double>>{
        interval<double>::closed(1, 3),
        interval<double>::closed(5, 7),
        interval<double>::closed(2, 4)  // Will be merged with [1, 3]
    };

    std::cout << "Automatic normalization maintains disjoint invariant\n";
    assert(set2.size() == 2);  // [1, 4] and [5, 7]

    // 2. Builder pattern for complex sets
    auto complex_set = disjoint_interval_set::disjoint_interval_set<interval<double>>::make()
        .add(1, 3)                    // Defaults to closed
        .add_open(5, 7)              // Open interval
        .add_singleton(10)           // Single point
        .add_left_open(12, 15)      // Half-open
        .build();

    std::cout << "Builder pattern provides clear, fluent construction\n";
    assert(complex_set.size() == 4);

    // 3. Named methods with clear semantics
    auto a = disjoint_interval_set::disjoint_interval_set<interval<double>>{
        interval<double>::closed(1, 5),
        interval<double>::closed(10, 15)
    };

    auto b = disjoint_interval_set::disjoint_interval_set<interval<double>>{
        interval<double>::closed(3, 7),
        interval<double>::closed(12, 18)
    };

    // All operations available as named methods
    auto union_set = a.unite(b);
    auto intersection_set = a.intersect(b);
    auto difference_set = a.difference(b);
    auto symmetric_diff = a.symmetric_difference(b);
    auto complement_set = a.complement(0, 20);

    std::cout << "Named methods make operations explicit and searchable\n";

    // 4. Operators for mathematical notation
    auto union_op = a + b;  // or a | b
    auto intersection_op = a * b;  // or a & b
    auto difference_op = a - b;
    auto symmetric_diff_op = a ^ b;
    auto complement_op = ~a;  // Universal complement

    assert(union_set == union_op);
    assert(intersection_set == intersection_op);
    assert(difference_set == difference_op);
    assert(symmetric_diff == symmetric_diff_op);

    std::cout << "Operators provide concise mathematical notation\n";

    // 5. Mutating operations for efficiency
    auto mutable_set = a;
    mutable_set.unite_with(b)         // Returns *this for chaining
               .intersect_with(a)
               .subtract(intersection_set);

    std::cout << "Mutating operations allow efficient in-place modifications\n";

    // 6. Rich query interface
    assert(a.contains(3.0));                     // Point membership
    assert(a.contains(interval<double>::closed(2, 4)));  // Interval containment
    assert(intersection_set.subset_of(a));       // Subset relation
    assert(a.overlaps_with(b));                  // Overlap test
    assert(!a.disjoint_with(b));                 // Disjoint test

    // 7. Range support for modern algorithms
    auto intervals = a.intervals();  // std::span<const interval<double>>
    assert(intervals.size() == a.size());

    // Works with range algorithms
    auto count = std::ranges::count_if(a, [](const auto& i) {
        return i.contains(3.0);
    });
    assert(count == 1);

    // 8. Boundary operations
    assert(a.infimum().value() == 1);
    assert(a.supremum().value() == 15);
    assert(a.lower_bound() == a.infimum());
    assert(a.upper_bound() == a.supremum());

    std::cout << "Comprehensive query interface with multiple naming conventions\n\n";
}

void demonstrate_composability() {
    std::cout << "=== Composability Improvements ===\n\n";

    // Intervals and sets compose naturally
    auto interval1 = interval<int>::closed(1, 10);
    auto interval2 = interval<int>::closed(5, 15);

    // Create a set from interval operations
    auto merged_interval = interval1.unite(interval2);
    if (merged_interval) {
        auto set = make_interval_set(*merged_interval);
        assert(set.size() == 1);
        std::cout << "Intervals compose into sets seamlessly\n";
    }

    // Sets can be built from various sources
    std::vector<interval<int>> interval_vec = {
        interval<int>::closed(1, 5),
        interval<int>::closed(10, 15),
        interval<int>::closed(20, 25)
    };

    auto set_from_range = disjoint_interval_set::disjoint_interval_set<interval<int>>(interval_vec);
    assert(set_from_range.size() == 3);

    // Operations can be chained fluently
    auto result = set_from_range
        .unite(make_interval_set(interval<int>::closed(7, 8)))
        .difference(make_interval_set(interval<int>::closed(22, 23)))
        .complement(0, 30);

    std::cout << "Operations chain naturally for complex expressions\n";

    // Type aliases make common use cases simple
    disjoint_interval_set::reals real_set;  // disjoint_interval_set<interval<double>>
    disjoint_interval_set::integers int_set;  // disjoint_interval_set<interval<int>>

    real_set = disjoint_interval_set::reals::make()
        .add(1.5, 3.7)
        .add(5.2, 8.9)
        .build();

    int_set = disjoint_interval_set::integers::singleton(42);

    std::cout << "Type aliases provide convenience without sacrificing clarity\n\n";
}

void demonstrate_performance_improvements() {
    std::cout << "=== Performance Improvements ===\n\n";

    // Move semantics prevent unnecessary copies
    auto create_large_set = []() {
        auto builder = disjoint_interval_set::disjoint_interval_set<interval<double>>::make();
        for (int i = 0; i < 1000; i += 2) {
            builder.add(i, i + 1);
        }
        return builder.build();  // Move, not copy
    };

    auto large_set = create_large_set();  // Move constructed
    assert(large_set.size() == 500);

    std::cout << "Move semantics eliminate unnecessary copies\n";

    // Binary search for efficient membership testing
    assert(large_set.contains(501.5));  // O(log n) lookup
    assert(!large_set.contains(501.5));

    // Reserve capacity for known sizes
    auto another_set = disjoint_interval_set::disjoint_interval_set<interval<double>>::make_empty();
    another_set.reserve(100);  // Pre-allocate space

    std::cout << "Efficient algorithms and memory management\n\n";
}

int main() {
    std::cout << "Demonstrating Improved Disjoint Interval Set API\n";
    std::cout << "================================================\n\n";

    demonstrate_interval_api();
    demonstrate_interval_set_api();
    demonstrate_composability();
    demonstrate_performance_improvements();

    std::cout << "All demonstrations completed successfully!\n";
    std::cout << "\nKey improvements:\n";
    std::cout << "- Clear, consistent naming (factory methods, named operations)\n";
    std::cout << "- Multiple ways to express operations (methods and operators)\n";
    std::cout << "- Builder pattern for complex construction\n";
    std::cout << "- Move semantics and performance optimizations\n";
    std::cout << "- Range support for modern C++ algorithms\n";
    std::cout << "- Composable components that work together naturally\n";

    return 0;
}