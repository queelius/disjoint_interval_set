#include <gtest/gtest.h>
#include "../include/dis/dis.hpp"
#include <vector>
#include <algorithm>

using namespace dis;

using real_set = disjoint_interval_set<interval<double>>;
using int_set = disjoint_interval_set<interval<int>>;
using real_interval = interval<double>;
using int_interval = interval<int>;

// ======================================================================
// CONSTRUCTION TESTS
// ======================================================================

TEST(ConstructionTest, DefaultConstruction) {
    real_set set;
    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0);
    ASSERT_FALSE(set.contains(0));
}

TEST(ConstructionTest, SingleIntervalConstruction) {
    auto interval = real_interval::closed(0, 10);
    real_set set(interval);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 1);
    ASSERT_TRUE(set.contains(5));
}

TEST(ConstructionTest, InitializerListConstruction) {
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
        real_interval{},
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

TEST(ConstructionTest, RangeConstruction) {
    std::vector<real_interval> intervals = {
        real_interval::closed(0, 10),
        real_interval::closed(20, 30),
        real_interval::closed(40, 50)
    };
    real_set set(intervals);
    ASSERT_EQ(set.size(), 3);
}

TEST(ConstructionTest, NamedConstructors) {
    // empty
    auto empty = real_set{};
    ASSERT_TRUE(empty.empty());

    // point
    auto point = real_set::point(5.5);
    ASSERT_EQ(point.size(), 1);
    ASSERT_TRUE(point.contains(5.5));
    ASSERT_FALSE(point.contains(5.49));

    // unbounded
    auto unbounded = real_set::unbounded();
    ASSERT_EQ(unbounded.size(), 1);
    ASSERT_TRUE(unbounded.contains(0));
    ASSERT_TRUE(unbounded.contains(1e100));
    ASSERT_TRUE(unbounded.contains(-1e100));
}

// ======================================================================
// QUERY TESTS
// ======================================================================

TEST(QueryTest, ContainmentValue) {
    auto set = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30),
        real_interval::open(40, 50)
    };

    // Points in intervals
    ASSERT_TRUE(set.contains(5));
    ASSERT_TRUE(set.contains(25));
    ASSERT_TRUE(set.contains(45));

    // Boundary points
    ASSERT_TRUE(set.contains(0));
    ASSERT_TRUE(set.contains(10));
    ASSERT_TRUE(set.contains(20));
    ASSERT_TRUE(set.contains(30));
    ASSERT_FALSE(set.contains(40));  // open interval
    ASSERT_FALSE(set.contains(50));  // open interval

    // Points outside intervals
    ASSERT_FALSE(set.contains(-1));
    ASSERT_FALSE(set.contains(15));
    ASSERT_FALSE(set.contains(35));
    ASSERT_FALSE(set.contains(60));
}

TEST(QueryTest, ContainmentInterval) {
    auto set = real_set{
        real_interval::closed(0, 20),
        real_interval::closed(30, 50)
    };

    // Contained intervals
    ASSERT_TRUE(set.contains(real_interval::closed(5, 15)));
    ASSERT_TRUE(set.contains(real_interval::closed(35, 45)));
    ASSERT_TRUE(set.contains(real_interval::point(10)));

    // Not contained
    ASSERT_FALSE(set.contains(real_interval::closed(15, 25)));  // spans gap
    ASSERT_FALSE(set.contains(real_interval::closed(25, 35)));  // spans gap
    ASSERT_FALSE(set.contains(real_interval::closed(60, 70)));  // outside

    // Empty interval
    ASSERT_TRUE(set.contains(real_interval{}));
}

TEST(QueryTest, SubsetSuperset) {
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

    ASSERT_TRUE(b.subset_of(a));
    ASSERT_FALSE(a.subset_of(b));
    ASSERT_FALSE(c.subset_of(a));  // c overlaps outside a

    ASSERT_TRUE(a.superset_of(b));
    ASSERT_FALSE(b.superset_of(a));
    ASSERT_FALSE(a.superset_of(c));

    // Self-relation
    ASSERT_TRUE(a.subset_of(a));
    ASSERT_TRUE(a.superset_of(a));

    // Empty set
    ASSERT_TRUE(real_set{}.subset_of(a));
    ASSERT_TRUE(a.superset_of(real_set{}));
}

TEST(QueryTest, Overlaps) {
    auto a = real_set{real_interval::closed(0, 10)};
    auto b = real_set{real_interval::closed(5, 15)};
    auto c = real_set{real_interval::closed(20, 30)};

    ASSERT_TRUE(a.overlaps(b));
    ASSERT_TRUE(b.overlaps(a));
    ASSERT_FALSE(a.overlaps(c));
    ASSERT_FALSE(c.overlaps(a));

    // Empty set doesn't overlap
    ASSERT_FALSE(a.overlaps(real_set{}));
}

// ======================================================================
// SET OPERATION TESTS
// ======================================================================

TEST(SetOperationTest, Union) {
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
    ASSERT_TRUE(result.contains(real_interval::closed(0, 15)));
    ASSERT_TRUE(result.contains(real_interval::closed(20, 35)));

    // Union with empty
    ASSERT_EQ(a | real_set{}, a);
    ASSERT_EQ(real_set{} | a, a);

    // Union with self
    ASSERT_EQ(a | a, a);

    // Test operator|=
    auto a_copy = a;
    a_copy |= b;
    ASSERT_EQ(a_copy, result);
}

TEST(SetOperationTest, Intersection) {
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
    ASSERT_TRUE(result.contains(real_interval::closed(5, 10)));
    ASSERT_TRUE(result.contains(real_interval::closed(25, 30)));

    // Intersection with empty
    ASSERT_EQ(a & real_set{}, real_set{});
    ASSERT_EQ(real_set{} & a, real_set{});

    // Intersection with self
    ASSERT_EQ(a & a, a);

    // Disjoint sets
    auto c = real_set{real_interval::closed(40, 50)};
    ASSERT_EQ(a & c, real_set{});

    // Test operator&=
    auto a_copy = a;
    a_copy &= b;
    ASSERT_EQ(a_copy, result);
}

TEST(SetOperationTest, Difference) {
    auto a = real_set{
        real_interval::closed(0, 20),
        real_interval::closed(30, 50)
    };
    auto b = real_set{
        real_interval::closed(10, 35)
    };

    auto result = a - b;  // difference
    ASSERT_EQ(result.size(), 2);
    ASSERT_TRUE(result.contains(real_interval::right_open(0, 10)));
    ASSERT_TRUE(result.contains(real_interval::left_open(35, 50)));

    // Difference with empty
    ASSERT_EQ(a - real_set{}, a);
    ASSERT_EQ(real_set{} - a, real_set{});

    // Difference with self
    ASSERT_EQ(a - a, real_set{});

    // Test operator-=
    auto a_copy = a;
    a_copy -= b;
    ASSERT_EQ(a_copy, result);
}

TEST(SetOperationTest, SymmetricDifference) {
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
    ASSERT_TRUE(result.contains(3));   // in a but not b
    ASSERT_FALSE(result.contains(7));  // in both
    ASSERT_TRUE(result.contains(12));  // in b but not a
    ASSERT_TRUE(result.contains(22));  // in a but not b
    ASSERT_FALSE(result.contains(27)); // in both
    ASSERT_TRUE(result.contains(33));  // in b but not a

    // Symmetric difference with empty
    ASSERT_EQ(a ^ real_set{}, a);
    ASSERT_EQ(real_set{} ^ a, a);

    // Symmetric difference with self
    ASSERT_EQ(a ^ a, real_set{});

    // Test operator^=
    auto a_copy = a;
    a_copy ^= b;
    ASSERT_EQ(a_copy, result);
}

TEST(SetOperationTest, Complement) {
    auto a = real_set{
        real_interval::closed(10, 20),
        real_interval::closed(30, 40)
    };

    auto comp = ~a;
    // Should contain everything except [10,20] and [30,40]
    ASSERT_TRUE(comp.contains(5));
    ASSERT_FALSE(comp.contains(15));
    ASSERT_TRUE(comp.contains(25));
    ASSERT_FALSE(comp.contains(35));
    ASSERT_TRUE(comp.contains(45));

    // Double complement
    ASSERT_EQ(~~a, a);

    // Complement of empty is unbounded
    auto comp_empty = ~real_set{};
    ASSERT_TRUE(comp_empty.contains(0));
    ASSERT_TRUE(comp_empty.contains(1e100));
    ASSERT_TRUE(comp_empty.contains(-1e100));
}

// ======================================================================
// FLUENT INTERFACE TESTS
// ======================================================================

TEST(FluentInterfaceTest, AddOperations) {
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

TEST(FluentInterfaceTest, RemoveOperations) {
    auto set = real_set{real_interval::closed(0, 50)};

    // Remove interval
    auto set2 = set.remove(real_interval::closed(20, 30));
    ASSERT_EQ(set2.size(), 2);
    ASSERT_TRUE(set2.contains(10));
    ASSERT_FALSE(set2.contains(25));
    ASSERT_TRUE(set2.contains(40));

    // Remove point interval (creates hole)
    auto set3 = set.remove(real_interval::point(25));
    ASSERT_FALSE(set3.contains(25));

    // Remove multiple intervals one by one
    auto set4 = set
        .remove(real_interval::closed(10, 15))
        .remove(real_interval::closed(35, 40));
    ASSERT_FALSE(set4.contains(12));
    ASSERT_FALSE(set4.contains(37));
}

// ======================================================================
// MEASURE AND QUERY TESTS
// ======================================================================

TEST(MeasureTest, Span) {
    auto set = real_set{
        real_interval::closed(10, 20),
        real_interval::closed(30, 40),
        real_interval::closed(50, 60)
    };

    auto span = set.span();
    ASSERT_EQ(span, real_interval::closed(10, 60));

    // Empty set has no span
    auto empty_span = real_set{}.span();
    ASSERT_TRUE(empty_span.empty());

    // Single interval span
    auto single = real_set{real_interval::closed(5, 15)};
    ASSERT_EQ(single.span(), real_interval::closed(5, 15));
}

TEST(MeasureTest, Gaps) {
    auto set = real_set{
        real_interval::closed(10, 20),
        real_interval::closed(30, 40),
        real_interval::closed(50, 60)
    };

    auto gaps = set.gaps();
    ASSERT_EQ(gaps.size(), 2);
    ASSERT_TRUE(gaps.contains(real_interval::open(20, 30)));
    ASSERT_TRUE(gaps.contains(real_interval::open(40, 50)));

    // No gaps in continuous set
    auto continuous = real_set{real_interval::closed(0, 100)};
    ASSERT_TRUE(continuous.gaps().empty());

    // No gaps in empty set
    ASSERT_TRUE(real_set{}.gaps().empty());
}

TEST(MeasureTest, Components) {
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

TEST(MeasureTest, Measures) {
    auto set = real_set{
        real_interval::closed(10, 20),  // length 10
        real_interval::closed(30, 50),  // length 20
        real_interval::closed(60, 70)   // length 10
    };

    ASSERT_NEAR(set.measure(), 40, 1e-10);
    ASSERT_NEAR(set.gap_measure(), 20, 1e-10);  // gaps: (20,30) and (50,60)
    ASSERT_NEAR(set.density(), 2.0/3.0, 1e-10);  // 40/60

    // Empty set measures
    ASSERT_EQ(real_set{}.measure(), 0);
    ASSERT_EQ(real_set{}.gap_measure(), 0);
    ASSERT_EQ(real_set{}.density(), 0);

    // Single interval
    auto single = real_set{real_interval::closed(0, 100)};
    ASSERT_NEAR(single.measure(), 100, 1e-10);
    ASSERT_NEAR(single.gap_measure(), 0, 1e-10);
    ASSERT_NEAR(single.density(), 1, 1e-10);
}

// ======================================================================
// FUNCTIONAL INTERFACE TESTS
// ======================================================================

TEST(FunctionalTest, Filter) {
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
    ASSERT_FALSE(filtered.contains(3));
    ASSERT_TRUE(filtered.contains(15));
    ASSERT_TRUE(filtered.contains(35));

    // Filter by position
    auto left_half = set.filter([](const auto& i) {
        return *i.upper_bound() <= 25;
    });
    ASSERT_EQ(left_half.size(), 2);
}

TEST(FunctionalTest, ForEach) {
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

    // Test gaps
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

TEST(FunctionalTest, Map) {
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
    ASSERT_TRUE(scaled.contains(real_interval::closed(2, 10)));
    ASSERT_TRUE(scaled.contains(real_interval::closed(20, 40)));
    ASSERT_TRUE(scaled.contains(real_interval::closed(60, 80)));

    // Shift all intervals
    auto shifted = set.map([](const auto& i) {
        return real_interval::closed(
            *i.lower_bound() + 100,
            *i.upper_bound() + 100
        );
    });

    ASSERT_TRUE(shifted.contains(105));
    ASSERT_TRUE(shifted.contains(115));
    ASSERT_TRUE(shifted.contains(135));
}

// ======================================================================
// ITERATOR TESTS
// ======================================================================

TEST(IteratorTest, BasicIteration) {
    auto set = real_set{
        real_interval::closed(1, 5),
        real_interval::closed(10, 20),
        real_interval::closed(30, 40)
    };

    // Range-based for loop
    int count = 0;
    for (const auto& interval : set) {
        count++;
        ASSERT_TRUE(interval.length() > 0);
    }
    ASSERT_EQ(count, 3);

    // Iterator operations
    auto it = set.begin();
    ASSERT_TRUE(it != set.end());
    ASSERT_EQ(*it, real_interval::closed(1, 5));

    ++it;
    ASSERT_EQ(*it, real_interval::closed(10, 20));

    ++it;
    ASSERT_EQ(*it, real_interval::closed(30, 40));

    ++it;
    ASSERT_TRUE(it == set.end());

    // Empty set iteration
    count = 0;
    for (const auto& interval : real_set{}) {
        count++;
    }
    ASSERT_EQ(count, 0);
}

// ======================================================================
// COMPARISON TESTS
// ======================================================================

TEST(ComparisonTest, Equality) {
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
    ASSERT_EQ(real_set{}, real_set{});
}

TEST(ComparisonTest, Ordering) {
    auto a = real_set{real_interval::closed(0, 10)};
    auto b = real_set{real_interval::closed(20, 30)};
    auto c = real_set{real_interval::closed(5, 15)};

    // Lexicographic ordering by first interval
    ASSERT_TRUE(a < b);
    ASSERT_FALSE(b < a);
    ASSERT_TRUE(a <= b);
    ASSERT_TRUE(b > a);
    ASSERT_TRUE(b >= a);

    // Overlapping intervals
    ASSERT_TRUE(a < c); // a's first interval starts before c's
}

// ======================================================================
// EDGE CASE TESTS
// ======================================================================

TEST(EdgeCaseTest, EmptySetOperations) {
    auto empty = real_set{};
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

TEST(EdgeCaseTest, SinglePointSets) {
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
    ASSERT_TRUE(intersect_diff.empty());
}

TEST(EdgeCaseTest, AdjacentIntervalsMerge) {
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

TEST(EdgeCaseTest, UnboundedIntervals) {
    auto unbounded = real_set::unbounded();
    ASSERT_EQ(unbounded.size(), 1);
    ASSERT_TRUE(unbounded.contains(0));
    ASSERT_TRUE(unbounded.contains(1e100));
    ASSERT_TRUE(unbounded.contains(-1e100));

    // Operations with unbounded
    auto bounded = real_set{real_interval::closed(0, 10)};
    ASSERT_EQ(unbounded | bounded, unbounded);
    ASSERT_EQ(unbounded & bounded, bounded);
    ASSERT_NE(unbounded - bounded, unbounded);

    // Complement of unbounded is empty
    ASSERT_EQ(~unbounded, real_set{});
}

TEST(EdgeCaseTest, LargeNumberOfIntervals) {
    // Create set with many intervals
    std::vector<real_interval> intervals;
    for (int i = 0; i < 1000; i++) {
        intervals.push_back(real_interval::closed(i * 10, i * 10 + 5));
    }

    real_set large_set(intervals);
    ASSERT_EQ(large_set.size(), 1000);
    ASSERT_EQ(large_set.component_count(), 1000);

    // Test contains
    ASSERT_TRUE(large_set.contains(42));    // in interval [40, 45]
    ASSERT_FALSE(large_set.contains(47));   // in gap (45, 50)

    // Test measure
    ASSERT_NEAR(large_set.measure(), 5000, 1e-10);  // 1000 intervals * 5 length each
}

// ======================================================================
// INTEGER INTERVAL SET TESTS
// ======================================================================

TEST(IntegerSetTest, BasicOperations) {
    auto set = int_set{
        int_interval::closed(1, 10),
        int_interval::closed(20, 30),
        int_interval::closed(40, 50)
    };

    ASSERT_EQ(set.size(), 3);
    ASSERT_TRUE(set.contains(5));
    ASSERT_TRUE(set.contains(25));
    ASSERT_FALSE(set.contains(15));

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
