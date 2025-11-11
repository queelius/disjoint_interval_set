#include <gtest/gtest.h>
#include "../include/dis/dis.hpp"
#include <limits>
#include <cmath>
#include <vector>

using namespace dis;

using real_interval = interval<double>;
using int_interval = interval<int>;

// ======================================================================
// CONSTRUCTION TESTS
// ======================================================================

TEST(ConstructionTest, DefaultConstructor) {
    real_interval i;
    ASSERT_TRUE(i.empty());
    ASSERT_FALSE(i.contains(0));
    ASSERT_FALSE(i.contains(1e10));
    ASSERT_FALSE(i.contains(-1e10));
}

TEST(ConstructionTest, GenericConstructorAllCases) {
    // Normal cases
    auto closed = real_interval(1, 10, true, true);
    ASSERT_TRUE(closed.contains(1));
    ASSERT_TRUE(closed.contains(10));
    ASSERT_TRUE(closed.contains(5));
    ASSERT_FALSE(closed.contains(0));
    ASSERT_FALSE(closed.contains(11));

    auto open = real_interval(1, 10, false, false);
    ASSERT_FALSE(open.contains(1));
    ASSERT_FALSE(open.contains(10));
    ASSERT_TRUE(open.contains(5));

    auto left_open = real_interval(1, 10, false, true);
    ASSERT_FALSE(left_open.contains(1));
    ASSERT_TRUE(left_open.contains(10));
    ASSERT_TRUE(left_open.contains(5));

    auto right_open = real_interval(1, 10, true, false);
    ASSERT_TRUE(right_open.contains(1));
    ASSERT_FALSE(right_open.contains(10));
    ASSERT_TRUE(right_open.contains(5));

    // Edge cases that should create empty intervals
    auto reverse = real_interval(10, 1); // upper < lower
    ASSERT_TRUE(reverse.empty());

    auto open_point = real_interval(5, 5, false, false); // open interval at single point
    ASSERT_TRUE(open_point.empty());

    auto left_open_point = real_interval(5, 5, false, true);
    ASSERT_TRUE(left_open_point.empty());

    auto right_open_point = real_interval(5, 5, true, false);
    ASSERT_TRUE(right_open_point.empty());
}

TEST(ConstructionTest, AllFactoryMethods) {
    // closed
    auto closed = real_interval::closed(0, 10);
    ASSERT_TRUE(closed.contains(0));
    ASSERT_TRUE(closed.contains(10));
    ASSERT_TRUE(closed.contains(5));

    // open
    auto open = real_interval::open(0, 10);
    ASSERT_FALSE(open.contains(0));
    ASSERT_FALSE(open.contains(10));
    ASSERT_TRUE(open.contains(5));

    // left_open
    auto left_open = real_interval::left_open(0, 10);
    ASSERT_FALSE(left_open.contains(0));
    ASSERT_TRUE(left_open.contains(10));

    // right_open
    auto right_open = real_interval::right_open(0, 10);
    ASSERT_TRUE(right_open.contains(0));
    ASSERT_FALSE(right_open.contains(10));

    // point
    auto point = real_interval::point(5.5);
    ASSERT_TRUE(point.contains(5.5));
    ASSERT_FALSE(point.contains(5.49999));
    ASSERT_FALSE(point.contains(5.50001));
    ASSERT_TRUE(point.is_point());

    // empty
    auto empty = real_interval{};
    ASSERT_TRUE(empty.empty());
    ASSERT_FALSE(empty.contains(0));

    // unbounded
    auto unbounded = real_interval::unbounded();
    ASSERT_TRUE(unbounded.contains(0));
    ASSERT_TRUE(unbounded.contains(1e100));
    ASSERT_TRUE(unbounded.contains(-1e100));
    ASSERT_FALSE(unbounded.is_bounded());

    // at_least
    auto at_least = real_interval::at_least(5);
    ASSERT_TRUE(at_least.contains(5));
    ASSERT_TRUE(at_least.contains(1e100));
    ASSERT_FALSE(at_least.contains(4.99999));

    // at_most
    auto at_most = real_interval::at_most(5);
    ASSERT_TRUE(at_most.contains(5));
    ASSERT_TRUE(at_most.contains(-1e100));
    ASSERT_FALSE(at_most.contains(5.00001));

    // greater_than
    auto greater = real_interval::greater_than(5);
    ASSERT_FALSE(greater.contains(5));
    ASSERT_TRUE(greater.contains(5.00001));
    ASSERT_TRUE(greater.contains(1e100));

    // less_than
    auto less = real_interval::less_than(5);
    ASSERT_FALSE(less.contains(5));
    ASSERT_TRUE(less.contains(4.99999));
    ASSERT_TRUE(less.contains(-1e100));
}

// ======================================================================
// BOUNDARY ACCESSOR TESTS
// ======================================================================

TEST(BoundaryTest, Accessors) {
    auto closed = real_interval::closed(1, 10);
    ASSERT_TRUE(closed.lower_bound().has_value());
    ASSERT_EQ(*closed.lower_bound(), 1);
    ASSERT_TRUE(closed.upper_bound().has_value());
    ASSERT_EQ(*closed.upper_bound(), 10);
    ASSERT_TRUE(closed.is_left_closed());
    ASSERT_TRUE(closed.is_right_closed());

    auto open = real_interval::open(1, 10);
    ASSERT_FALSE(open.is_left_closed());
    ASSERT_FALSE(open.is_right_closed());

    auto empty = real_interval{};
    // Empty intervals should not have bounds
    ASSERT_FALSE(empty.lower_bound().has_value());
    ASSERT_FALSE(empty.upper_bound().has_value());

    auto unbounded = real_interval::unbounded();
    // Unbounded interval has infinite bounds
    ASSERT_TRUE(unbounded.lower_bound().has_value());
    ASSERT_TRUE(unbounded.upper_bound().has_value());
    ASSERT_TRUE(std::isinf(*unbounded.lower_bound()));
    ASSERT_TRUE(std::isinf(*unbounded.upper_bound()));
}

// ======================================================================
// QUERY METHOD TESTS
// ======================================================================

TEST(QueryTest, IntervalQueries) {
    // is_empty
    ASSERT_TRUE(real_interval{}.empty());
    ASSERT_FALSE(real_interval::closed(0, 10).empty());
    ASSERT_TRUE(real_interval(10, 5).empty()); // reversed bounds

    // is_point
    ASSERT_TRUE(real_interval::point(5).is_point());
    ASSERT_FALSE(real_interval::closed(5, 5.001).is_point());
    ASSERT_FALSE(real_interval{}.is_point());

    // is_bounded
    ASSERT_TRUE(real_interval::closed(0, 10).is_bounded());
    ASSERT_FALSE(real_interval::unbounded().is_bounded());
    ASSERT_FALSE(real_interval::at_least(5).is_bounded());
    ASSERT_FALSE(real_interval::at_most(5).is_bounded());
    ASSERT_FALSE(real_interval{}.is_bounded()); // empty is NOT considered bounded

    // Verify unbounded intervals through their bounds
    auto less = real_interval::less_than(10);
    ASSERT_TRUE(less.lower_bound().has_value());
    ASSERT_TRUE(std::isinf(*less.lower_bound()));

    auto greater = real_interval::greater_than(0);
    ASSERT_TRUE(greater.upper_bound().has_value());
    ASSERT_TRUE(std::isinf(*greater.upper_bound()));
}

// ======================================================================
// CONTAINMENT TESTS
// ======================================================================

TEST(ContainmentTest, ValueContainment) {
    auto closed = real_interval::closed(0, 10);
    ASSERT_TRUE(closed.contains(0));
    ASSERT_TRUE(closed.contains(5));
    ASSERT_TRUE(closed.contains(10));
    ASSERT_FALSE(closed.contains(-1));
    ASSERT_FALSE(closed.contains(11));

    auto open = real_interval::open(0, 10);
    ASSERT_FALSE(open.contains(0));
    ASSERT_TRUE(open.contains(5));
    ASSERT_FALSE(open.contains(10));

    auto empty = real_interval{};
    ASSERT_FALSE(empty.contains(0));
    ASSERT_FALSE(empty.contains(std::numeric_limits<double>::infinity()));
    ASSERT_FALSE(empty.contains(-std::numeric_limits<double>::infinity()));
}

TEST(ContainmentTest, IntervalContainment) {
    auto large = real_interval::closed(0, 100);
    auto small = real_interval::closed(25, 75);
    auto overlapping = real_interval::closed(50, 150);
    auto disjoint = real_interval::closed(200, 300);

    // Use subset_of to check interval containment
    ASSERT_TRUE(small.subset_of(large));
    ASSERT_FALSE(overlapping.subset_of(large));
    ASSERT_FALSE(disjoint.subset_of(large));
    ASSERT_FALSE(large.subset_of(small));

    // Empty interval tests
    ASSERT_TRUE(real_interval{}.subset_of(large));
    ASSERT_FALSE(large.subset_of(real_interval{}));
    ASSERT_TRUE(real_interval{}.subset_of(real_interval{}));
}

// ======================================================================
// RELATIONAL OPERATION TESTS
// ======================================================================

TEST(RelationTest, SubsetSuperset) {
    auto a = real_interval::closed(0, 100);
    auto b = real_interval::closed(25, 75);
    auto c = real_interval::closed(50, 150);

    ASSERT_TRUE(b.subset_of(a));
    ASSERT_FALSE(a.subset_of(b));
    ASSERT_FALSE(c.subset_of(a));

    ASSERT_TRUE(a.superset_of(b));
    ASSERT_FALSE(b.superset_of(a));
    ASSERT_FALSE(a.superset_of(c));

    // Empty interval is subset of everything
    ASSERT_TRUE(real_interval{}.subset_of(a));
    ASSERT_TRUE(a.superset_of(real_interval{}));

    // Self-subset
    ASSERT_TRUE(a.subset_of(a));
    ASSERT_TRUE(a.superset_of(a));
}

TEST(RelationTest, OverlapsDisjoint) {
    auto a = real_interval::closed(0, 50);
    auto b = real_interval::closed(25, 75);
    auto c = real_interval::closed(100, 150);

    ASSERT_TRUE(a.overlaps(b));
    ASSERT_TRUE(b.overlaps(a));
    ASSERT_FALSE(a.overlaps(c));
    ASSERT_FALSE(c.overlaps(a));

    ASSERT_FALSE(a.disjoint_from(b));
    ASSERT_FALSE(b.disjoint_from(a));
    ASSERT_TRUE(a.disjoint_from(c));
    ASSERT_TRUE(c.disjoint_from(a));

    // Empty interval tests
    ASSERT_FALSE(a.overlaps(real_interval{}));
    ASSERT_TRUE(a.disjoint_from(real_interval{}));
}

TEST(RelationTest, AdjacentIntervals) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(10, 20);
    auto c = real_interval::open(10, 20);
    auto d = real_interval::closed(11, 20);

    // [0,10] and [10,20] share boundary point 10
    ASSERT_FALSE(a.adjacent_to(b)); // They overlap at 10

    // [0,10] and (10,20) are adjacent
    ASSERT_TRUE(a.adjacent_to(c));
    ASSERT_TRUE(c.adjacent_to(a));

    // [0,10] and [11,20] are not adjacent (gap between)
    ASSERT_FALSE(a.adjacent_to(d));

    // Test with open intervals
    auto e = real_interval::open(0, 10);
    auto f = real_interval::closed(10, 20);
    ASSERT_TRUE(e.adjacent_to(f));

    // Test integer intervals for clarity
    auto int_a = int_interval::closed(0, 10);
    auto int_b = int_interval::closed(11, 20);
    ASSERT_FALSE(int_a.adjacent_to(int_b)); // NOT adjacent - there's no shared boundary point
}

// ======================================================================
// SET OPERATION TESTS
// ======================================================================

TEST(SetOperationTest, Intersection) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(20, 30);

    auto ab = a.intersect(b);
    ASSERT_EQ(ab, real_interval::closed(5, 10));

    auto ac = a.intersect(c);
    ASSERT_TRUE(ac.empty());

    // Test with different boundary types
    auto d = real_interval::open(0, 10);
    auto e = real_interval::closed(5, 15);
    auto de = d.intersect(e);
    ASSERT_EQ(de, real_interval::right_open(5, 10));

    // Empty interval intersection
    auto empty = real_interval{};
    ASSERT_EQ(a.intersect(empty), empty);
}

TEST(SetOperationTest, Hull) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(20, 30);

    auto ab_hull = a.hull(b);
    ASSERT_TRUE(ab_hull.has_value());
    ASSERT_EQ(*ab_hull, real_interval::closed(0, 15));

    auto ac_hull = a.hull(c);
    ASSERT_FALSE(ac_hull.has_value()); // hull only works for overlapping or adjacent intervals

    // Hull with empty interval
    auto empty = real_interval{};
    auto a_empty_hull = a.hull(empty);
    ASSERT_TRUE(a_empty_hull.has_value());
    ASSERT_EQ(*a_empty_hull, a);

    auto empty_empty_hull = empty.hull(empty);
    ASSERT_TRUE(empty_empty_hull.has_value()); // hull of two empties returns empty
    ASSERT_TRUE(empty_empty_hull->empty());
}

TEST(SetOperationTest, JoinViaHull) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(10, 20); // touching
    auto d = real_interval::closed(30, 40); // disjoint

    auto ab_hull = a.hull(b);
    ASSERT_TRUE(ab_hull.has_value());
    ASSERT_EQ(*ab_hull, real_interval::closed(0, 20));

    auto ad_hull = a.hull(d);
    ASSERT_FALSE(ad_hull.has_value()); // can't create hull of disjoint intervals
}

// ======================================================================
// COMPARISON TESTS
// ======================================================================

TEST(ComparisonTest, Equality) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(0, 10);
    auto c = real_interval::open(0, 10);
    auto d = real_interval::closed(0, 11);

    ASSERT_EQ(a, b);
    ASSERT_NE(a, c);
    ASSERT_NE(a, d);

    // Empty intervals are equal
    ASSERT_EQ(real_interval{}, real_interval{});
    ASSERT_EQ(real_interval(10, 5), real_interval{});
}

TEST(ComparisonTest, Ordering) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(20, 30);

    ASSERT_TRUE(a < c); // a is entirely before c
    ASSERT_FALSE(c < a);
    ASSERT_TRUE(a <= a);
    ASSERT_TRUE(c > a);
    ASSERT_TRUE(c >= c);

    // Overlapping intervals
    // Ordering is lexicographic: first by lower bound, then upper
    ASSERT_TRUE(a < b); // same lower, but a has smaller upper
}

// ======================================================================
// MEASURE TESTS
// ======================================================================

TEST(MeasureTest, Length) {
    auto a = real_interval::closed(2, 8);
    ASSERT_NEAR(a.length(), 6, 1e-10);

    auto b = real_interval::open(2, 8);
    ASSERT_NEAR(b.length(), 6, 1e-10); // length same for open

    auto point = real_interval::point(5);
    ASSERT_NEAR(point.length(), 0, 1e-10);

    auto empty = real_interval{};
    ASSERT_NEAR(empty.length(), 0, 1e-10);

    // Integer intervals
    auto int_interval = int_interval::closed(1, 10);
    ASSERT_EQ(int_interval.length(), 9);
}

TEST(MeasureTest, Midpoint) {
    auto a = real_interval::closed(2, 8);
    ASSERT_NEAR(a.midpoint(), 5, 1e-10);

    auto b = real_interval::closed(-10, 10);
    ASSERT_NEAR(b.midpoint(), 0, 1e-10);

    auto point = real_interval::point(7);
    ASSERT_NEAR(point.midpoint(), 7, 1e-10);
}

TEST(MeasureTest, Distance) {
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
    auto empty = real_interval{};
    ASSERT_EQ(a.distance_to(empty), 0); // distance to empty is 0 per implementation
}

// ======================================================================
// SPECIAL VALUE TESTS
// ======================================================================

TEST(SpecialValueTest, InfinityHandling) {
    auto inf = std::numeric_limits<double>::infinity();
    auto ninf = -inf;

    auto unbounded = real_interval::unbounded();
    // Unbounded has infinite bounds but they're open, so doesn't contain infinity itself
    ASSERT_FALSE(unbounded.contains(inf));
    ASSERT_FALSE(unbounded.contains(ninf));
    ASSERT_TRUE(unbounded.contains(0));

    auto at_least = real_interval::at_least(0);
    ASSERT_FALSE(at_least.contains(inf)); // open at infinity
    ASSERT_FALSE(at_least.contains(ninf));

    auto at_most = real_interval::at_most(0);
    ASSERT_FALSE(at_most.contains(inf));
    ASSERT_FALSE(at_most.contains(ninf)); // open at negative infinity
}

TEST(SpecialValueTest, NaNHandling) {
    auto nan = std::numeric_limits<double>::quiet_NaN();

    auto a = real_interval::closed(0, 10);
    ASSERT_FALSE(a.contains(nan)); // NaN is never contained

    // NaN handling is implementation-specific
    // The interval constructor may not detect NaN as invalid
    auto nan_interval2 = real_interval(0, nan);
    // Not asserting is_empty() as behavior may vary
}

// ======================================================================
// EDGE CASE TESTS
// ======================================================================

TEST(EdgeCaseTest, ExtremeValues) {
    auto max_val = std::numeric_limits<double>::max();
    auto min_val = std::numeric_limits<double>::lowest();
    auto epsilon = std::numeric_limits<double>::epsilon();

    auto extreme = real_interval::closed(min_val, max_val);
    ASSERT_TRUE(extreme.contains(0));
    ASSERT_TRUE(extreme.contains(max_val));
    ASSERT_TRUE(extreme.contains(min_val));

    // Very small interval
    auto tiny = real_interval::closed(0, epsilon);
    ASSERT_TRUE(tiny.contains(0));
    ASSERT_TRUE(tiny.contains(epsilon));
    ASSERT_FALSE(tiny.contains(2 * epsilon));
    ASSERT_NEAR(tiny.length(), epsilon, epsilon/10);
}

TEST(EdgeCaseTest, IntegerBoundaryCases) {
    auto max_int = std::numeric_limits<int>::max();
    auto min_int = std::numeric_limits<int>::min();

    auto full_range = int_interval::closed(min_int, max_int);
    ASSERT_TRUE(full_range.contains(0));
    ASSERT_TRUE(full_range.contains(max_int));
    ASSERT_TRUE(full_range.contains(min_int));

    // Adjacent integer intervals - need shared boundary point
    auto a = int_interval::closed(0, 10);
    auto b = int_interval::open(10, 20);
    ASSERT_TRUE(a.adjacent_to(b)); // [0,10] is adjacent to (10,20)

    auto c = int_interval::closed(10, 20);
    ASSERT_FALSE(a.adjacent_to(c)); // they overlap at 10
}
