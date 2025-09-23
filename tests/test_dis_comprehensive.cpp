/**
 * Comprehensive unit tests for the disjoint_interval_set class
 * Goal: Achieve >95% code coverage for disjoint_interval_set.hpp
 */

#include "../include/dis/dis.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <set>
#include <random>
#include <chrono>

using namespace dis;

// Test framework
int tests_run = 0;
int tests_passed = 0;

#define TEST(name) void name(); \
    struct name##_runner { \
        name##_runner() { \
            std::cout << "Running " #name "... "; \
            tests_run++; \
            try { \
                name(); \
                std::cout << "PASSED\n"; \
                tests_passed++; \
            } catch (const std::exception& e) { \
                std::cout << "FAILED: " << e.what() << '\n'; \
            } \
        } \
    } name##_instance; \
    void name()

#define ASSERT(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    }

#define ASSERT_EQ(a, b) \
    if (!((a) == (b))) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << (a) << " != " << (b); \
        throw std::runtime_error(oss.str()); \
    }

#define ASSERT_NE(a, b) \
    if ((a) == (b)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << (a) << " == " << (b); \
        throw std::runtime_error(oss.str()); \
    }

#define ASSERT_NEAR(a, b, epsilon) \
    if (std::abs((a) - (b)) > (epsilon)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << (a) << " not near " << (b); \
        throw std::runtime_error(oss.str()); \
    }

using real_set = disjoint_interval_set<interval<double>>;
using int_set = disjoint_interval_set<interval<int>>;
using real_interval = interval<double>;
using int_interval = interval<int>;

// ======================================================================
// CONSTRUCTION TESTS
// ======================================================================

TEST(test_default_construction) {
    real_set set;
    ASSERT(set.is_empty());
    ASSERT_EQ(set.size(), 0);
    ASSERT(!set.contains(0));
}

TEST(test_single_interval_construction) {
    auto interval = real_interval::closed(0, 10);
    real_set set(interval);
    ASSERT(!set.is_empty());
    ASSERT_EQ(set.size(), 1);
    ASSERT(set.contains(5));
}

TEST(test_initializer_list_construction) {
    // Non-overlapping intervals
    real_set set1{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30),
        real_interval::closed(40, 50)
    };
    ASSERT_EQ(set1.size(), 3);

    // Overlapping intervals - should merge
    real_set set2{
        real_interval::closed(0, 10),
        real_interval::closed(5, 15),  // Overlaps with first
        real_interval::closed(25, 35)
    };
    ASSERT_EQ(set2.size(), 2); // First two should merge

    // Empty intervals should be ignored
    real_set set3{
        real_interval::closed(0, 10),
        real_interval::empty(),
        real_interval::closed(20, 30)
    };
    ASSERT_EQ(set3.size(), 2);

    // Adjacent intervals should merge
    real_set set4{
        real_interval::closed(0, 10),
        real_interval::closed(10, 20),  // Shares point 10
        real_interval::closed(20, 30)   // Shares point 20
    };
    ASSERT_EQ(set4.size(), 1); // All should merge
}

TEST(test_range_construction) {
    std::vector<real_interval> intervals = {
        real_interval::closed(0, 10),
        real_interval::closed(20, 30),
        real_interval::closed(40, 50)
    };
    real_set set(intervals);
    ASSERT_EQ(set.size(), 3);
}

TEST(test_named_constructors) {
    // empty
    auto empty = real_set::empty();
    ASSERT(empty.is_empty());

    // point
    auto point = real_set::point(5.5);
    ASSERT_EQ(point.size(), 1);
    ASSERT(point.contains(5.5));
    ASSERT(!point.contains(5.49));

    // unbounded
    auto unbounded = real_set::unbounded();
    ASSERT_EQ(unbounded.size(), 1);
    ASSERT(unbounded.contains(0));
    ASSERT(unbounded.contains(1e100));
    ASSERT(unbounded.contains(-1e100));
}

// ======================================================================
// QUERY TESTS
// ======================================================================

TEST(test_containment_value) {
    auto set = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30),
        real_interval::open(40, 50)
    };

    // Points in intervals
    ASSERT(set.contains(5));
    ASSERT(set.contains(25));
    ASSERT(set.contains(45));

    // Boundary points
    ASSERT(set.contains(0));
    ASSERT(set.contains(10));
    ASSERT(set.contains(20));
    ASSERT(set.contains(30));
    ASSERT(!set.contains(40));  // open interval
    ASSERT(!set.contains(50));  // open interval

    // Points outside intervals
    ASSERT(!set.contains(-1));
    ASSERT(!set.contains(15));
    ASSERT(!set.contains(35));
    ASSERT(!set.contains(60));
}

TEST(test_containment_interval) {
    auto set = real_set{
        real_interval::closed(0, 20),
        real_interval::closed(30, 50)
    };

    // Contained intervals
    ASSERT(set.contains(real_interval::closed(5, 15)));
    ASSERT(set.contains(real_interval::closed(35, 45)));
    ASSERT(set.contains(real_interval::point(10)));

    // Not contained
    ASSERT(!set.contains(real_interval::closed(15, 25)));  // spans gap
    ASSERT(!set.contains(real_interval::closed(25, 35)));  // spans gap
    ASSERT(!set.contains(real_interval::closed(60, 70)));  // outside

    // Empty interval
    ASSERT(set.contains(real_interval::empty()));
}

TEST(test_subset_superset) {
    auto a = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto b = real_set{
        real_interval::closed(2, 8),
        real_interval::closed(22, 28)
    };
    auto c = real_set{
        real_interval::closed(0, 15)
    };

    ASSERT(b.subset_of(a));
    ASSERT(!a.subset_of(b));
    ASSERT(!c.subset_of(a));  // c overlaps outside a

    ASSERT(a.superset_of(b));
    ASSERT(!b.superset_of(a));
    ASSERT(!a.superset_of(c));

    // Self-relation
    ASSERT(a.subset_of(a));
    ASSERT(a.superset_of(a));

    // Empty set
    ASSERT(real_set::empty().subset_of(a));
    ASSERT(a.superset_of(real_set::empty()));
}

TEST(test_overlaps) {
    auto a = real_set{real_interval::closed(0, 10)};
    auto b = real_set{real_interval::closed(5, 15)};
    auto c = real_set{real_interval::closed(20, 30)};

    ASSERT(a.overlaps(b));
    ASSERT(b.overlaps(a));
    ASSERT(!a.overlaps(c));
    ASSERT(!c.overlaps(a));

    // Empty set doesn't overlap
    ASSERT(!a.overlaps(real_set::empty()));
}

// ======================================================================
// SET OPERATION TESTS
// ======================================================================

TEST(test_union_operation) {
    auto a = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto b = real_set{
        real_interval::closed(5, 15),
        real_interval::closed(25, 35)
    };

    auto result = a | b;  // union
    ASSERT_EQ(result.size(), 2);
    ASSERT(result.contains(real_interval::closed(0, 15)));
    ASSERT(result.contains(real_interval::closed(20, 35)));

    // Union with empty
    ASSERT_EQ(a | real_set::empty(), a);
    ASSERT_EQ(real_set::empty() | a, a);

    // Union with self
    ASSERT_EQ(a | a, a);

    // Test operator|=
    auto a_copy = a;
    a_copy |= b;
    ASSERT_EQ(a_copy, result);
}

TEST(test_intersection_operation) {
    auto a = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto b = real_set{
        real_interval::closed(5, 15),
        real_interval::closed(25, 35)
    };

    auto result = a & b;  // intersection
    ASSERT_EQ(result.size(), 2);
    ASSERT(result.contains(real_interval::closed(5, 10)));
    ASSERT(result.contains(real_interval::closed(25, 30)));

    // Intersection with empty
    ASSERT_EQ(a & real_set::empty(), real_set::empty());
    ASSERT_EQ(real_set::empty() & a, real_set::empty());

    // Intersection with self
    ASSERT_EQ(a & a, a);

    // Disjoint sets
    auto c = real_set{real_interval::closed(40, 50)};
    ASSERT_EQ(a & c, real_set::empty());

    // Test operator&=
    auto a_copy = a;
    a_copy &= b;
    ASSERT_EQ(a_copy, result);
}

TEST(test_difference_operation) {
    auto a = real_set{
        real_interval::closed(0, 20),
        real_interval::closed(30, 50)
    };
    auto b = real_set{
        real_interval::closed(10, 35)
    };

    auto result = a - b;  // difference
    ASSERT_EQ(result.size(), 2);
    ASSERT(result.contains(real_interval::right_open(0, 10)));
    ASSERT(result.contains(real_interval::left_open(35, 50)));

    // Difference with empty
    ASSERT_EQ(a - real_set::empty(), a);
    ASSERT_EQ(real_set::empty() - a, real_set::empty());

    // Difference with self
    ASSERT_EQ(a - a, real_set::empty());

    // Test operator-=
    auto a_copy = a;
    a_copy -= b;
    ASSERT_EQ(a_copy, result);
}

TEST(test_symmetric_difference) {
    auto a = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto b = real_set{
        real_interval::closed(5, 15),
        real_interval::closed(25, 35)
    };

    auto result = a ^ b;  // symmetric difference
    // Should contain parts of a not in b, and parts of b not in a
    ASSERT(result.contains(3));   // in a but not b
    ASSERT(!result.contains(7));  // in both
    ASSERT(result.contains(12));  // in b but not a
    ASSERT(result.contains(22));  // in a but not b
    ASSERT(!result.contains(27)); // in both
    ASSERT(result.contains(33));  // in b but not a

    // Symmetric difference with empty
    ASSERT_EQ(a ^ real_set::empty(), a);
    ASSERT_EQ(real_set::empty() ^ a, a);

    // Symmetric difference with self
    ASSERT_EQ(a ^ a, real_set::empty());

    // Test operator^=
    auto a_copy = a;
    a_copy ^= b;
    ASSERT_EQ(a_copy, result);
}

TEST(test_complement) {
    auto a = real_set{
        real_interval::closed(10, 20),
        real_interval::closed(30, 40)
    };

    auto comp = ~a;
    // Should contain everything except [10,20] and [30,40]
    ASSERT(comp.contains(5));
    ASSERT(!comp.contains(15));
    ASSERT(comp.contains(25));
    ASSERT(!comp.contains(35));
    ASSERT(comp.contains(45));

    // Double complement
    ASSERT_EQ(~~a, a);

    // Complement of empty is unbounded
    auto comp_empty = ~real_set::empty();
    ASSERT(comp_empty.contains(0));
    ASSERT(comp_empty.contains(1e100));
    ASSERT(comp_empty.contains(-1e100));
}

// ======================================================================
// FLUENT INTERFACE TESTS
// ======================================================================

TEST(test_add_operations) {
    auto set = real_set{}
        .add(0, 10)
        .add(5, 15)  // overlaps, should merge
        .add(20, 30)
        .add(40, 50);

    ASSERT_EQ(set.size(), 3); // First two should merge

    // Add interval object
    auto set2 = real_set{}.add(real_interval::closed(0, 10));
    ASSERT_EQ(set2.size(), 1);

    // Add multiple intervals one by one
    auto set3 = set
        .add(real_interval::closed(60, 70))
        .add(real_interval::closed(80, 90));
    ASSERT_EQ(set3.size(), 5);
}

TEST(test_remove_operations) {
    auto set = real_set{real_interval::closed(0, 50)};

    // Remove interval
    auto set2 = set.remove(real_interval::closed(20, 30));
    ASSERT_EQ(set2.size(), 2);
    ASSERT(set2.contains(10));
    ASSERT(!set2.contains(25));
    ASSERT(set2.contains(40));

    // Remove point interval (creates hole)
    auto set3 = set.remove(real_interval::point(25));
    ASSERT(!set3.contains(25));
    // Point removal may not create a precise hole due to floating point

    // Remove multiple intervals one by one
    auto set4 = set
        .remove(real_interval::closed(10, 15))
        .remove(real_interval::closed(35, 40));
    ASSERT(!set4.contains(12));
    ASSERT(!set4.contains(37));
}

// clear() method doesn't exist - removed this test

// ======================================================================
// MEASURE AND QUERY TESTS
// ======================================================================

TEST(test_span) {
    auto set = real_set{
        real_interval::closed(10, 20),
        real_interval::closed(30, 40),
        real_interval::closed(50, 60)
    };

    auto span = set.span();
    ASSERT_EQ(span, real_interval::closed(10, 60));

    // Empty set has no span
    auto empty_span = real_set::empty().span();
    ASSERT(empty_span.is_empty());

    // Single interval span
    auto single = real_set{real_interval::closed(5, 15)};
    ASSERT_EQ(single.span(), real_interval::closed(5, 15));
}

TEST(test_gaps) {
    auto set = real_set{
        real_interval::closed(10, 20),
        real_interval::closed(30, 40),
        real_interval::closed(50, 60)
    };

    auto gaps = set.gaps();
    ASSERT_EQ(gaps.size(), 2);
    ASSERT(gaps.contains(real_interval::open(20, 30)));
    ASSERT(gaps.contains(real_interval::open(40, 50)));

    // No gaps in continuous set
    auto continuous = real_set{real_interval::closed(0, 100)};
    ASSERT(continuous.gaps().is_empty());

    // No gaps in empty set
    ASSERT(real_set::empty().gaps().is_empty());
}

TEST(test_components) {
    auto set = real_set{
        real_interval::closed(10, 20),
        real_interval::closed(30, 40),
        real_interval::closed(50, 60)
    };

    auto components = set.components();
    ASSERT_EQ(components.size(), 3);
    ASSERT_EQ(components[0], real_interval::closed(10, 20));
    ASSERT_EQ(components[1], real_interval::closed(30, 40));
    ASSERT_EQ(components[2], real_interval::closed(50, 60));

    // Test component_count
    ASSERT_EQ(set.component_count(), 3);
}

TEST(test_measures) {
    auto set = real_set{
        real_interval::closed(10, 20),  // length 10
        real_interval::closed(30, 50),  // length 20
        real_interval::closed(60, 70)   // length 10
    };

    ASSERT_NEAR(set.measure(), 40, 1e-10);
    ASSERT_NEAR(set.gap_measure(), 20, 1e-10);  // gaps: (20,30) and (50,60)
    ASSERT_NEAR(set.density(), 2.0/3.0, 1e-10);  // 40/60

    // Empty set measures
    ASSERT_EQ(real_set::empty().measure(), 0);
    ASSERT_EQ(real_set::empty().gap_measure(), 0);
    ASSERT_EQ(real_set::empty().density(), 0);

    // Single interval
    auto single = real_set{real_interval::closed(0, 100)};
    ASSERT_NEAR(single.measure(), 100, 1e-10);
    ASSERT_NEAR(single.gap_measure(), 0, 1e-10);
    ASSERT_NEAR(single.density(), 1, 1e-10);
}

// ======================================================================
// FUNCTIONAL INTERFACE TESTS
// ======================================================================

TEST(test_filter) {
    auto set = real_set{
        real_interval::closed(1, 5),    // length 4
        real_interval::closed(10, 20),  // length 10
        real_interval::closed(30, 45)   // length 15
    };

    // Filter by length
    auto filtered = set.filter([](const auto& i) {
        return i.length() >= 10;
    });
    ASSERT_EQ(filtered.size(), 2);
    ASSERT(!filtered.contains(3));
    ASSERT(filtered.contains(15));
    ASSERT(filtered.contains(35));

    // Filter by position
    auto left_half = set.filter([](const auto& i) {
        return *i.upper_bound() <= 25;
    });
    ASSERT_EQ(left_half.size(), 2);
}

TEST(test_for_each) {
    auto set = real_set{
        real_interval::closed(1, 5),
        real_interval::closed(10, 20),
        real_interval::closed(30, 40)
    };

    int count = 0;
    double total_length = 0;
    set.for_each([&](const auto& i) {
        count++;
        total_length += i.length();
    });

    ASSERT_EQ(count, 3);
    ASSERT_NEAR(total_length, 24, 1e-10);

    // for_each_gap doesn't exist - test gaps() instead
    auto gaps = set.gaps();
    count = 0;
    total_length = 0;
    gaps.for_each([&](const auto& gap) {
        count++;
        total_length += gap.length();
    });

    ASSERT_EQ(count, 2);
    ASSERT_NEAR(total_length, 15, 1e-10); // gaps: (5,10) and (20,30)
}

TEST(test_map) {
    auto set = real_set{
        real_interval::closed(1, 5),
        real_interval::closed(10, 20),
        real_interval::closed(30, 40)
    };

    // Scale all intervals by 2
    auto scaled = set.map([](const auto& i) {
        return real_interval::closed(
            *i.lower_bound() * 2,
            *i.upper_bound() * 2
        );
    });

    ASSERT_EQ(scaled.size(), 3);
    ASSERT(scaled.contains(real_interval::closed(2, 10)));
    ASSERT(scaled.contains(real_interval::closed(20, 40)));
    ASSERT(scaled.contains(real_interval::closed(60, 80)));

    // Shift all intervals
    auto shifted = set.map([](const auto& i) {
        return real_interval::closed(
            *i.lower_bound() + 100,
            *i.upper_bound() + 100
        );
    });

    ASSERT(shifted.contains(105));
    ASSERT(shifted.contains(115));
    ASSERT(shifted.contains(135));
}

// reduce() method doesn't exist - removed this test

// any/all/none methods don't exist - removed this test

// ======================================================================
// ITERATOR TESTS
// ======================================================================

TEST(test_iterators) {
    auto set = real_set{
        real_interval::closed(1, 5),
        real_interval::closed(10, 20),
        real_interval::closed(30, 40)
    };

    // Range-based for loop
    int count = 0;
    for (const auto& interval : set) {
        count++;
        ASSERT(interval.length() > 0);
    }
    ASSERT_EQ(count, 3);

    // Iterator operations
    auto it = set.begin();
    ASSERT(it != set.end());
    ASSERT_EQ(*it, real_interval::closed(1, 5));

    ++it;
    ASSERT_EQ(*it, real_interval::closed(10, 20));

    ++it;
    ASSERT_EQ(*it, real_interval::closed(30, 40));

    ++it;
    ASSERT(it == set.end());

    // Empty set iteration
    count = 0;
    for (const auto& interval : real_set::empty()) {
        count++;
    }
    ASSERT_EQ(count, 0);
}

// ======================================================================
// COMPARISON TESTS
// ======================================================================

TEST(test_equality_comparison) {
    auto a = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto b = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto c = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 31)  // Different
    };

    ASSERT_EQ(a, b);
    ASSERT_NE(a, c);
    ASSERT_NE(b, c);

    // Empty sets are equal
    ASSERT_EQ(real_set::empty(), real_set::empty());
}

TEST(test_ordering_comparison) {
    auto a = real_set{real_interval::closed(0, 10)};
    auto b = real_set{real_interval::closed(20, 30)};
    auto c = real_set{real_interval::closed(5, 15)};

    // Lexicographic ordering by first interval
    ASSERT(a < b);
    ASSERT(!(b < a));
    ASSERT(a <= b);
    ASSERT(b > a);
    ASSERT(b >= a);

    // Overlapping intervals
    ASSERT(a < c); // a's first interval starts before c's
}

// ======================================================================
// EDGE CASE TESTS
// ======================================================================

TEST(test_empty_set_operations) {
    auto empty = real_set::empty();
    auto non_empty = real_set{real_interval::closed(0, 10)};

    // All operations with empty set
    ASSERT_EQ(empty | empty, empty);
    ASSERT_EQ(empty & empty, empty);
    ASSERT_EQ(empty - empty, empty);
    ASSERT_EQ(empty ^ empty, empty);

    ASSERT_EQ(empty | non_empty, non_empty);
    ASSERT_EQ(non_empty | empty, non_empty);

    ASSERT_EQ(empty & non_empty, empty);
    ASSERT_EQ(non_empty & empty, empty);

    ASSERT_EQ(empty - non_empty, empty);
    ASSERT_EQ(non_empty - empty, non_empty);

    ASSERT_EQ(empty ^ non_empty, non_empty);
    ASSERT_EQ(non_empty ^ empty, non_empty);
}

TEST(test_single_point_sets) {
    auto point1 = real_set::point(5);
    auto point2 = real_set::point(10);
    auto point3 = real_set::point(5);

    ASSERT_EQ(point1, point3);
    ASSERT_NE(point1, point2);

    auto union_points = point1 | point2;
    ASSERT_EQ(union_points.size(), 2);

    auto intersect_same = point1 & point3;
    ASSERT_EQ(intersect_same, point1);

    auto intersect_diff = point1 & point2;
    ASSERT(intersect_diff.is_empty());
}

TEST(test_adjacent_intervals_merge) {
    // Adjacent closed intervals that share a point should merge
    auto set1 = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(10, 20)
    };
    ASSERT_EQ(set1.size(), 1);
    ASSERT_EQ(set1.components()[0], real_interval::closed(0, 20));

    // Adjacent intervals that don't share a point should not merge
    auto set2 = real_set{
        real_interval::right_open(0, 10),
        real_interval::left_open(10, 20)
    };
    ASSERT_EQ(set2.size(), 2);
}

TEST(test_unbounded_intervals) {
    auto unbounded = real_set::unbounded();
    ASSERT_EQ(unbounded.size(), 1);
    ASSERT(unbounded.contains(0));
    ASSERT(unbounded.contains(1e100));
    ASSERT(unbounded.contains(-1e100));

    // Operations with unbounded
    auto bounded = real_set{real_interval::closed(0, 10)};
    ASSERT_EQ(unbounded | bounded, unbounded);
    ASSERT_EQ(unbounded & bounded, bounded);
    ASSERT_NE(unbounded - bounded, unbounded);

    // Complement of unbounded is empty
    ASSERT_EQ(~unbounded, real_set::empty());
}

TEST(test_large_number_of_intervals) {
    // Create set with many intervals
    std::vector<real_interval> intervals;
    for (int i = 0; i < 1000; i++) {
        intervals.push_back(real_interval::closed(i * 10, i * 10 + 5));
    }

    real_set large_set(intervals);
    ASSERT_EQ(large_set.size(), 1000);
    ASSERT_EQ(large_set.component_count(), 1000);

    // Test contains
    ASSERT(large_set.contains(42));    // in interval [40, 45]
    ASSERT(!large_set.contains(47));   // in gap (45, 50)

    // Test measure
    ASSERT_NEAR(large_set.measure(), 5000, 1e-10);  // 1000 intervals * 5 length each
}

// ======================================================================
// INTEGER INTERVAL SET TESTS
// ======================================================================

TEST(test_integer_sets) {
    auto set = int_set{
        int_interval::closed(1, 10),
        int_interval::closed(20, 30),
        int_interval::closed(40, 50)
    };

    ASSERT_EQ(set.size(), 3);
    ASSERT(set.contains(5));
    ASSERT(set.contains(25));
    ASSERT(!set.contains(15));

    // Integer-specific adjacent intervals
    auto adjacent_set = int_set{
        int_interval::closed(1, 10),
        int_interval::closed(11, 20)  // Adjacent but not overlapping
    };
    ASSERT_EQ(adjacent_set.size(), 2);  // Should not merge

    // Test measures with integers
    // Integer intervals [1,10], [20,30], [40,50] have lengths 9, 10, 10
    ASSERT_EQ(set.measure(), 29);  // (10-1) + (30-20) + (50-40) = 9 + 10 + 10
}

// Main test runner
int main() {
    std::cout << "\n================================\n";
    std::cout << "   Comprehensive DIS Tests\n";
    std::cout << "================================\n\n";

    // Tests are automatically run by their constructors

    std::cout << "\n================================\n";
    std::cout << "Results: " << tests_passed << "/" << tests_run << " tests passed\n";

    if (tests_passed == tests_run) {
        std::cout << "All tests PASSED!\n";
        std::cout << "================================\n";
        return 0;
    } else {
        std::cout << "Some tests FAILED\n";
        std::cout << "================================\n";
        return 1;
    }
}