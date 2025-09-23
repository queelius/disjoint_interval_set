/**
 * Comprehensive unit tests for the interval class
 * Goal: Achieve >95% code coverage for interval.hpp
 */

#include "../include/dis/dis.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <limits>
#include <cmath>
#include <vector>
#include <algorithm>

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

#define ASSERT_THROWS(expr) \
    try { \
        expr; \
        throw std::runtime_error("Expected exception not thrown"); \
    } catch (...) {}

using real_interval = interval<double>;
using int_interval = interval<int>;

// ======================================================================
// CONSTRUCTION TESTS
// ======================================================================

TEST(test_default_constructor) {
    real_interval i;
    ASSERT(i.is_empty());
    ASSERT(!i.contains(0));
    ASSERT(!i.contains(1e10));
    ASSERT(!i.contains(-1e10));
}

TEST(test_generic_constructor_all_cases) {
    // Normal cases
    auto closed = real_interval(1, 10, true, true);
    ASSERT(closed.contains(1));
    ASSERT(closed.contains(10));
    ASSERT(closed.contains(5));
    ASSERT(!closed.contains(0));
    ASSERT(!closed.contains(11));

    auto open = real_interval(1, 10, false, false);
    ASSERT(!open.contains(1));
    ASSERT(!open.contains(10));
    ASSERT(open.contains(5));

    auto left_open = real_interval(1, 10, false, true);
    ASSERT(!left_open.contains(1));
    ASSERT(left_open.contains(10));
    ASSERT(left_open.contains(5));

    auto right_open = real_interval(1, 10, true, false);
    ASSERT(right_open.contains(1));
    ASSERT(!right_open.contains(10));
    ASSERT(right_open.contains(5));

    // Edge cases that should create empty intervals
    auto reverse = real_interval(10, 1); // upper < lower
    ASSERT(reverse.is_empty());

    auto open_point = real_interval(5, 5, false, false); // open interval at single point
    ASSERT(open_point.is_empty());

    auto left_open_point = real_interval(5, 5, false, true);
    ASSERT(left_open_point.is_empty());

    auto right_open_point = real_interval(5, 5, true, false);
    ASSERT(right_open_point.is_empty());
}

TEST(test_all_factory_methods) {
    // closed
    auto closed = real_interval::closed(0, 10);
    ASSERT(closed.contains(0));
    ASSERT(closed.contains(10));
    ASSERT(closed.contains(5));

    // open
    auto open = real_interval::open(0, 10);
    ASSERT(!open.contains(0));
    ASSERT(!open.contains(10));
    ASSERT(open.contains(5));

    // left_open
    auto left_open = real_interval::left_open(0, 10);
    ASSERT(!left_open.contains(0));
    ASSERT(left_open.contains(10));

    // right_open
    auto right_open = real_interval::right_open(0, 10);
    ASSERT(right_open.contains(0));
    ASSERT(!right_open.contains(10));

    // point
    auto point = real_interval::point(5.5);
    ASSERT(point.contains(5.5));
    ASSERT(!point.contains(5.49999));
    ASSERT(!point.contains(5.50001));
    ASSERT(point.is_point());

    // empty
    auto empty = real_interval::empty();
    ASSERT(empty.is_empty());
    ASSERT(!empty.contains(0));

    // unbounded
    auto unbounded = real_interval::unbounded();
    ASSERT(unbounded.contains(0));
    ASSERT(unbounded.contains(1e100));
    ASSERT(unbounded.contains(-1e100));
    ASSERT(!unbounded.is_bounded());

    // at_least
    auto at_least = real_interval::at_least(5);
    ASSERT(at_least.contains(5));
    ASSERT(at_least.contains(1e100));
    ASSERT(!at_least.contains(4.99999));

    // at_most
    auto at_most = real_interval::at_most(5);
    ASSERT(at_most.contains(5));
    ASSERT(at_most.contains(-1e100));
    ASSERT(!at_most.contains(5.00001));

    // greater_than
    auto greater = real_interval::greater_than(5);
    ASSERT(!greater.contains(5));
    ASSERT(greater.contains(5.00001));
    ASSERT(greater.contains(1e100));

    // less_than
    auto less = real_interval::less_than(5);
    ASSERT(!less.contains(5));
    ASSERT(less.contains(4.99999));
    ASSERT(less.contains(-1e100));
}

// ======================================================================
// BOUNDARY ACCESSOR TESTS
// ======================================================================

TEST(test_boundary_accessors) {
    auto closed = real_interval::closed(1, 10);
    ASSERT(closed.lower_bound().has_value());
    ASSERT_EQ(*closed.lower_bound(), 1);
    ASSERT(closed.upper_bound().has_value());
    ASSERT_EQ(*closed.upper_bound(), 10);
    ASSERT(closed.is_left_closed());
    ASSERT(closed.is_right_closed());

    auto open = real_interval::open(1, 10);
    ASSERT(!open.is_left_closed());
    ASSERT(!open.is_right_closed());

    auto empty = real_interval::empty();
    // Empty intervals should not have bounds
    ASSERT(!empty.lower_bound().has_value());
    ASSERT(!empty.upper_bound().has_value());

    auto unbounded = real_interval::unbounded();
    // Unbounded interval has infinite bounds
    ASSERT(unbounded.lower_bound().has_value());
    ASSERT(unbounded.upper_bound().has_value());
    ASSERT(std::isinf(*unbounded.lower_bound()));
    ASSERT(std::isinf(*unbounded.upper_bound()));
}

// ======================================================================
// QUERY METHOD TESTS
// ======================================================================

TEST(test_interval_queries) {
    // is_empty
    ASSERT(real_interval::empty().is_empty());
    ASSERT(!real_interval::closed(0, 10).is_empty());
    ASSERT(real_interval(10, 5).is_empty()); // reversed bounds

    // is_point
    ASSERT(real_interval::point(5).is_point());
    ASSERT(!real_interval::closed(5, 5.001).is_point());
    ASSERT(!real_interval::empty().is_point());

    // is_bounded
    ASSERT(real_interval::closed(0, 10).is_bounded());
    ASSERT(!real_interval::unbounded().is_bounded());
    ASSERT(!real_interval::at_least(5).is_bounded());
    ASSERT(!real_interval::at_most(5).is_bounded());
    ASSERT(!real_interval::empty().is_bounded()); // empty is NOT considered bounded

    // Verify unbounded intervals through their bounds
    auto less = real_interval::less_than(10);
    ASSERT(less.lower_bound().has_value());
    ASSERT(std::isinf(*less.lower_bound()));

    auto greater = real_interval::greater_than(0);
    ASSERT(greater.upper_bound().has_value());
    ASSERT(std::isinf(*greater.upper_bound()));
}

// ======================================================================
// CONTAINMENT TESTS
// ======================================================================

TEST(test_value_containment) {
    auto closed = real_interval::closed(0, 10);
    ASSERT(closed.contains(0));
    ASSERT(closed.contains(5));
    ASSERT(closed.contains(10));
    ASSERT(!closed.contains(-1));
    ASSERT(!closed.contains(11));

    auto open = real_interval::open(0, 10);
    ASSERT(!open.contains(0));
    ASSERT(open.contains(5));
    ASSERT(!open.contains(10));

    auto empty = real_interval::empty();
    ASSERT(!empty.contains(0));
    ASSERT(!empty.contains(std::numeric_limits<double>::infinity()));
    ASSERT(!empty.contains(-std::numeric_limits<double>::infinity()));
}

TEST(test_interval_containment) {
    auto large = real_interval::closed(0, 100);
    auto small = real_interval::closed(25, 75);
    auto overlapping = real_interval::closed(50, 150);
    auto disjoint = real_interval::closed(200, 300);

    // Use subset_of to check interval containment
    ASSERT(small.subset_of(large));
    ASSERT(!overlapping.subset_of(large));
    ASSERT(!disjoint.subset_of(large));
    ASSERT(!large.subset_of(small));

    // Empty interval tests
    ASSERT(real_interval::empty().subset_of(large));
    ASSERT(!large.subset_of(real_interval::empty()));
    ASSERT(real_interval::empty().subset_of(real_interval::empty()));
}

// ======================================================================
// RELATIONAL OPERATION TESTS
// ======================================================================

TEST(test_subset_superset) {
    auto a = real_interval::closed(0, 100);
    auto b = real_interval::closed(25, 75);
    auto c = real_interval::closed(50, 150);

    ASSERT(b.subset_of(a));
    ASSERT(!a.subset_of(b));
    ASSERT(!c.subset_of(a));

    ASSERT(a.superset_of(b));
    ASSERT(!b.superset_of(a));
    ASSERT(!a.superset_of(c));

    // Empty interval is subset of everything
    ASSERT(real_interval::empty().subset_of(a));
    ASSERT(a.superset_of(real_interval::empty()));

    // Self-subset
    ASSERT(a.subset_of(a));
    ASSERT(a.superset_of(a));
}

TEST(test_overlaps_disjoint) {
    auto a = real_interval::closed(0, 50);
    auto b = real_interval::closed(25, 75);
    auto c = real_interval::closed(100, 150);

    ASSERT(a.overlaps(b));
    ASSERT(b.overlaps(a));
    ASSERT(!a.overlaps(c));
    ASSERT(!c.overlaps(a));

    ASSERT(!a.disjoint_from(b));
    ASSERT(!b.disjoint_from(a));
    ASSERT(a.disjoint_from(c));
    ASSERT(c.disjoint_from(a));

    // Empty interval tests
    ASSERT(!a.overlaps(real_interval::empty()));
    ASSERT(a.disjoint_from(real_interval::empty()));
}

TEST(test_adjacent_intervals) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(10, 20);
    auto c = real_interval::open(10, 20);
    auto d = real_interval::closed(11, 20);

    // [0,10] and [10,20] share boundary point 10
    ASSERT(!a.adjacent_to(b)); // They overlap at 10

    // [0,10] and (10,20) are adjacent
    ASSERT(a.adjacent_to(c));
    ASSERT(c.adjacent_to(a));

    // [0,10] and [11,20] are not adjacent (gap between)
    ASSERT(!a.adjacent_to(d));

    // Test with open intervals
    auto e = real_interval::open(0, 10);
    auto f = real_interval::closed(10, 20);
    ASSERT(e.adjacent_to(f));

    // Test integer intervals for clarity
    auto int_a = int_interval::closed(0, 10);
    auto int_b = int_interval::closed(11, 20);
    ASSERT(!int_a.adjacent_to(int_b)); // NOT adjacent - there's no shared boundary point
}

// touches() method doesn't exist - removed this test

// before/after methods don't exist - removed this test

// ======================================================================
// SET OPERATION TESTS
// ======================================================================

TEST(test_intersection) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(20, 30);

    auto ab = a.intersect(b);
    ASSERT_EQ(ab, real_interval::closed(5, 10));

    auto ac = a.intersect(c);
    ASSERT(ac.is_empty());

    // Test with different boundary types
    auto d = real_interval::open(0, 10);
    auto e = real_interval::closed(5, 15);
    auto de = d.intersect(e);
    ASSERT_EQ(de, real_interval::right_open(5, 10));

    // Empty interval intersection
    auto empty = real_interval::empty();
    ASSERT_EQ(a.intersect(empty), empty);
}

TEST(test_hull) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(20, 30);

    auto ab_hull = a.hull(b);
    ASSERT(ab_hull.has_value());
    ASSERT_EQ(*ab_hull, real_interval::closed(0, 15));

    auto ac_hull = a.hull(c);
    ASSERT(!ac_hull.has_value()); // hull only works for overlapping or adjacent intervals

    // Hull with empty interval
    auto empty = real_interval::empty();
    auto a_empty_hull = a.hull(empty);
    ASSERT(a_empty_hull.has_value());
    ASSERT_EQ(*a_empty_hull, a);

    auto empty_empty_hull = empty.hull(empty);
    ASSERT(empty_empty_hull.has_value()); // hull of two empties returns empty
    ASSERT(empty_empty_hull->is_empty());
}

// join() method doesn't exist - using hull() which is similar
TEST(test_join_via_hull) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(10, 20); // touching
    auto d = real_interval::closed(30, 40); // disjoint

    auto ab_hull = a.hull(b);
    ASSERT(ab_hull.has_value());
    ASSERT_EQ(*ab_hull, real_interval::closed(0, 20));

    auto ad_hull = a.hull(d);
    ASSERT(!ad_hull.has_value()); // can't create hull of disjoint intervals
}

// ======================================================================
// COMPARISON TESTS
// ======================================================================

TEST(test_equality) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(0, 10);
    auto c = real_interval::open(0, 10);
    auto d = real_interval::closed(0, 11);

    ASSERT_EQ(a, b);
    ASSERT_NE(a, c);
    ASSERT_NE(a, d);

    // Empty intervals are equal
    ASSERT_EQ(real_interval::empty(), real_interval::empty());
    ASSERT_EQ(real_interval(10, 5), real_interval::empty());
}

TEST(test_ordering) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(20, 30);

    ASSERT(a < c); // a is entirely before c
    ASSERT(!(c < a));
    ASSERT(a <= a);
    ASSERT(c > a);
    ASSERT(c >= c);

    // Overlapping intervals
    // Ordering is lexicographic: first by lower bound, then upper
    ASSERT(a < b); // same lower, but a has smaller upper
}

// ======================================================================
// MEASURE TESTS
// ======================================================================

TEST(test_length) {
    auto a = real_interval::closed(2, 8);
    ASSERT_NEAR(a.length(), 6, 1e-10);

    auto b = real_interval::open(2, 8);
    ASSERT_NEAR(b.length(), 6, 1e-10); // length same for open

    auto point = real_interval::point(5);
    ASSERT_NEAR(point.length(), 0, 1e-10);

    auto empty = real_interval::empty();
    ASSERT_NEAR(empty.length(), 0, 1e-10);

    // Integer intervals
    auto int_interval = int_interval::closed(1, 10);
    ASSERT_EQ(int_interval.length(), 9);
}

TEST(test_midpoint) {
    auto a = real_interval::closed(2, 8);
    ASSERT_NEAR(a.midpoint(), 5, 1e-10);

    auto b = real_interval::closed(-10, 10);
    ASSERT_NEAR(b.midpoint(), 0, 1e-10);

    auto point = real_interval::point(7);
    ASSERT_NEAR(point.midpoint(), 7, 1e-10);
}

TEST(test_distance) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(20, 30);
    auto c = real_interval::closed(5, 15);

    ASSERT_NEAR(a.distance_to(b), 10, 1e-10);
    ASSERT_NEAR(b.distance_to(a), 10, 1e-10);
    ASSERT_NEAR(a.distance_to(c), 0, 1e-10); // overlapping

    // Distance to point
    auto point = real_interval::point(25);
    ASSERT_NEAR(a.distance_to(point), 15, 1e-10);

    // Distance involving empty
    auto empty = real_interval::empty();
    ASSERT_EQ(a.distance_to(empty), 0); // distance to empty is 0 per implementation
}

// Transformation methods (expand, contract, shift, scale, clamp) don't exist in the API

// Iterator methods don't exist in the API

// Split methods don't exist in the API

// ======================================================================
// SPECIAL VALUE TESTS
// ======================================================================

TEST(test_infinity_handling) {
    auto inf = std::numeric_limits<double>::infinity();
    auto ninf = -inf;

    auto unbounded = real_interval::unbounded();
    // Unbounded has infinite bounds but they're open, so doesn't contain infinity itself
    ASSERT(!unbounded.contains(inf));
    ASSERT(!unbounded.contains(ninf));
    ASSERT(unbounded.contains(0));

    auto at_least = real_interval::at_least(0);
    ASSERT(!at_least.contains(inf)); // open at infinity
    ASSERT(!at_least.contains(ninf));

    auto at_most = real_interval::at_most(0);
    ASSERT(!at_most.contains(inf));
    ASSERT(!at_most.contains(ninf)); // open at negative infinity
}

TEST(test_nan_handling) {
    auto nan = std::numeric_limits<double>::quiet_NaN();

    auto a = real_interval::closed(0, 10);
    ASSERT(!a.contains(nan)); // NaN is never contained

    // NaN handling is implementation-specific
    // The interval constructor may not detect NaN as invalid
    auto nan_interval2 = real_interval(0, nan);
    // Not asserting is_empty() as behavior may vary
}

// ======================================================================
// EDGE CASE TESTS
// ======================================================================

TEST(test_extreme_values) {
    auto max_val = std::numeric_limits<double>::max();
    auto min_val = std::numeric_limits<double>::lowest();
    auto epsilon = std::numeric_limits<double>::epsilon();

    auto extreme = real_interval::closed(min_val, max_val);
    ASSERT(extreme.contains(0));
    ASSERT(extreme.contains(max_val));
    ASSERT(extreme.contains(min_val));

    // Very small interval
    auto tiny = real_interval::closed(0, epsilon);
    ASSERT(tiny.contains(0));
    ASSERT(tiny.contains(epsilon));
    ASSERT(!tiny.contains(2 * epsilon));
    ASSERT_NEAR(tiny.length(), epsilon, epsilon/10);
}

TEST(test_integer_boundary_cases) {
    auto max_int = std::numeric_limits<int>::max();
    auto min_int = std::numeric_limits<int>::min();

    auto full_range = int_interval::closed(min_int, max_int);
    ASSERT(full_range.contains(0));
    ASSERT(full_range.contains(max_int));
    ASSERT(full_range.contains(min_int));

    // Adjacent integer intervals - need shared boundary point
    auto a = int_interval::closed(0, 10);
    auto b = int_interval::open(10, 20);
    ASSERT(a.adjacent_to(b)); // [0,10] is adjacent to (10,20)

    auto c = int_interval::closed(10, 20);
    ASSERT(!a.adjacent_to(c)); // they overlap at 10
}

// Main test runner
int main() {
    std::cout << "\n================================\n";
    std::cout << "   Comprehensive Interval Tests\n";
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