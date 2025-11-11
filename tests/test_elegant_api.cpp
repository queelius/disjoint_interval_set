#include <gtest/gtest.h>
#include "../include/dis/dis.hpp"
#include <cmath>
#include <sstream>

using namespace dis;

// === Interval Tests ===

TEST(IntervalTest, Construction) {
    auto closed = real_interval::closed(0, 10);
    ASSERT_TRUE(closed.contains(0));
    ASSERT_TRUE(closed.contains(10));
    ASSERT_TRUE(closed.contains(5));
    ASSERT_FALSE(closed.contains(-1));
    ASSERT_FALSE(closed.contains(11));

    auto open = real_interval::open(0, 10);
    ASSERT_FALSE(open.contains(0));
    ASSERT_FALSE(open.contains(10));
    ASSERT_TRUE(open.contains(5));

    auto point = real_interval::point(5);
    ASSERT_TRUE(point.contains(5));
    ASSERT_FALSE(point.contains(4.999));
    ASSERT_FALSE(point.contains(5.001));
    ASSERT_TRUE(point.is_point());

    auto empty = real_interval{};  // Default constructor creates empty interval
    ASSERT_TRUE(empty.empty());
    ASSERT_FALSE(empty.contains(0));
}

TEST(IntervalTest, Relations) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(2, 8);

    ASSERT_TRUE(c.subset_of(a));
    ASSERT_FALSE(a.subset_of(c));
    ASSERT_TRUE(a.superset_of(c));

    ASSERT_TRUE(a.overlaps(b));
    ASSERT_TRUE(b.overlaps(a));
    ASSERT_FALSE(a.disjoint_from(b));

    auto d = real_interval::closed(20, 30);
    ASSERT_TRUE(a.disjoint_from(d));
    ASSERT_FALSE(a.overlaps(d));

    auto e = real_interval::open(10, 20);
    ASSERT_TRUE(a.adjacent_to(e));
    ASSERT_FALSE(a.overlaps(e));
}

TEST(IntervalTest, Operations) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);

    auto intersection = a & b;
    ASSERT_EQ(intersection, real_interval::closed(5, 10));

    auto hull = a.hull(b);
    ASSERT_TRUE(hull.has_value());
    ASSERT_EQ(*hull, real_interval::closed(0, 15));

    auto c = real_interval::closed(20, 30);
    auto no_hull = a.hull(c);
    ASSERT_FALSE(no_hull.has_value());
}

TEST(IntervalTest, Measures) {
    auto interval = real_interval::closed(2, 8);
    ASSERT_NEAR(interval.length(), 6, 1e-10);
    ASSERT_NEAR(interval.midpoint(), 5, 1e-10);

    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(20, 30);
    ASSERT_NEAR(a.distance_to(b), 10, 1e-10);
}

// === Disjoint Interval Set Tests ===

TEST(DisjointIntervalSetTest, Construction) {
    auto set1 = real_set{};
    ASSERT_TRUE(set1.empty());

    auto set2 = real_set{real_interval::closed(0, 10)};
    ASSERT_FALSE(set2.empty());
    ASSERT_EQ(set2.size(), 1);

    auto set3 = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(5, 15),  // Overlaps - will merge
        real_interval::closed(20, 30)
    };
    ASSERT_EQ(set3.size(), 2);  // Should merge first two
}

TEST(DisjointIntervalSetTest, Containment) {
    auto set = real_set{}
        .add(0, 10)
        .add(20, 30)
        .add(40, 50);

    ASSERT_TRUE(set.contains(5));
    ASSERT_TRUE(set.contains(25));
    ASSERT_FALSE(set.contains(15));
    ASSERT_FALSE(set.contains(35));

    ASSERT_TRUE(set.contains(real_interval::closed(2, 8)));
    ASSERT_FALSE(set.contains(real_interval::closed(5, 25)));
}

TEST(DisjointIntervalSetTest, SetOperations) {
    auto a = real_set{}.add(0, 10).add(20, 30);
    auto b = real_set{}.add(5, 15).add(25, 35);

    auto union_ab = a | b;
    ASSERT_EQ(union_ab.size(), 2);
    ASSERT_TRUE(union_ab.contains(real_interval::closed(0, 15)));
    ASSERT_TRUE(union_ab.contains(real_interval::closed(20, 35)));

    auto intersect_ab = a & b;
    ASSERT_EQ(intersect_ab.size(), 2);
    ASSERT_TRUE(intersect_ab.contains(real_interval::closed(5, 10)));
    ASSERT_TRUE(intersect_ab.contains(real_interval::closed(25, 30)));

    auto diff_ab = a - b;
    ASSERT_EQ(diff_ab.size(), 2);
    ASSERT_TRUE(diff_ab.contains(real_interval::right_open(0, 5)));
    ASSERT_TRUE(diff_ab.contains(real_interval::right_open(20, 25)));
}

TEST(DisjointIntervalSetTest, Queries) {
    auto set = real_set{}.add(10, 20).add(30, 40).add(50, 60);

    auto span = set.span();
    ASSERT_EQ(span, real_interval::closed(10, 60));

    auto gaps = set.gaps();
    ASSERT_EQ(gaps.size(), 2);
    ASSERT_TRUE(gaps.contains(real_interval::open(20, 30)));
    ASSERT_TRUE(gaps.contains(real_interval::open(40, 50)));

    ASSERT_EQ(set.component_count(), 3);

    ASSERT_NEAR(set.measure(), 30, 1e-10);
    ASSERT_NEAR(set.gap_measure(), 20, 1e-10);
    ASSERT_NEAR(set.density(), 0.6, 1e-10);
}

TEST(DisjointIntervalSetTest, FluentInterface) {
    auto set = real_set{}
        .add(0, 10)
        .add(5, 15)  // Will merge with [0, 10]
        .add(20, 30)
        .remove(real_interval::closed(12, 25))
        .add(40, 50);

    ASSERT_EQ(set.size(), 3);
    ASSERT_TRUE(set.contains(real_interval::closed(0, 11)));
    ASSERT_TRUE(set.contains(real_interval::closed(26, 30)));
    ASSERT_TRUE(set.contains(real_interval::closed(40, 50)));
}

TEST(DisjointIntervalSetTest, FunctionalOperations) {
    auto set = real_set{}.add(1, 5).add(10, 20).add(30, 40);

    auto filtered = set.filter([](const auto& i) {
        return i.length() >= 10;
    });
    ASSERT_EQ(filtered.size(), 2);

    int count = 0;
    set.for_each([&count](const auto&) { count++; });
    ASSERT_EQ(count, 3);

    auto scaled = set.map([](const auto& i) {
        return real_interval::closed(
            *i.lower_bound() * 2,
            *i.upper_bound() * 2
        );
    });
    ASSERT_TRUE(scaled.contains(2));
    ASSERT_TRUE(scaled.contains(40));
    ASSERT_TRUE(scaled.contains(80));
}

// === Parser Tests ===

TEST(ParserTest, IntervalParsing) {
    auto i1 = parse_interval<double>("[0, 10]");
    ASSERT_EQ(i1, real_interval::closed(0, 10));

    auto i2 = parse_interval<double>("(0, 10)");
    ASSERT_EQ(i2, real_interval::open(0, 10));

    auto i3 = parse_interval<double>("[0, 10)");
    ASSERT_EQ(i3, real_interval::right_open(0, 10));

    auto i4 = parse_interval<double>("{5}");
    ASSERT_EQ(i4, real_interval::point(5));

    auto i5 = parse_interval<double>("{}");
    ASSERT_EQ(i5, real_interval{});
}

TEST(ParserTest, SetParsing) {
    auto set1 = real_set::from_string("[0,10] U [20,30]");
    ASSERT_EQ(set1.size(), 2);

    auto set2 = real_set::from_string("[0,10) U (20,30] U {50}");
    ASSERT_EQ(set2.size(), 3);

    auto empty = real_set::from_string("{}");
    ASSERT_TRUE(empty.empty());
}

// === Formatter Tests ===

TEST(FormatterTest, BasicFormatting) {
    auto interval = real_interval::closed(0, 10);
    auto formatted = interval_formatter<double>::format(
        interval, interval_formatter<double>::Style::Mathematical);
    ASSERT_EQ(formatted, "[0,10]");

    auto set = real_set{}.add(0, 10).add(20, 30);
    auto set_formatted = interval_formatter<double>::format(
        set, interval_formatter<double>::Style::Mathematical);
    // Should be "[0,10] U [20,30]"
    ASSERT_FALSE(set_formatted.empty());
}

// === Edge Cases ===

TEST(EdgeCasesTest, EmptyOperations) {
    // Empty operations
    auto empty = real_set{};
    auto non_empty = real_set{real_interval::closed(0, 10)};

    ASSERT_EQ(empty | non_empty, non_empty);
    ASSERT_EQ(empty & non_empty, empty);
    ASSERT_TRUE(empty.subset_of(non_empty));

    // Single point operations
    auto point_set = real_set::point(5);
    ASSERT_TRUE(point_set.contains(5));
    ASSERT_FALSE(point_set.contains(4.999));

    // Adjacent intervals
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::open(10, 20);
    ASSERT_TRUE(a.adjacent_to(b));

    auto set_with_adjacent = real_set{a, b};
    ASSERT_EQ(set_with_adjacent.size(), 1);  // Should merge adjacent
}
