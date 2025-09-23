#include "../include/dis/dis.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <sstream>

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

#define ASSERT_NEAR(a, b, epsilon) \
    if (std::abs((a) - (b)) > (epsilon)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << (a) << " not near " << (b); \
        throw std::runtime_error(oss.str()); \
    }

// === Interval Tests ===

TEST(test_interval_construction) {
    auto closed = real_interval::closed(0, 10);
    ASSERT(closed.contains(0));
    ASSERT(closed.contains(10));
    ASSERT(closed.contains(5));
    ASSERT(!closed.contains(-1));
    ASSERT(!closed.contains(11));

    auto open = real_interval::open(0, 10);
    ASSERT(!open.contains(0));
    ASSERT(!open.contains(10));
    ASSERT(open.contains(5));

    auto point = real_interval::point(5);
    ASSERT(point.contains(5));
    ASSERT(!point.contains(4.999));
    ASSERT(!point.contains(5.001));
    ASSERT(point.is_point());

    auto empty = real_interval::empty();
    ASSERT(empty.is_empty());
    ASSERT(!empty.contains(0));
}

TEST(test_interval_relations) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);
    auto c = real_interval::closed(2, 8);

    ASSERT(c.subset_of(a));
    ASSERT(!a.subset_of(c));
    ASSERT(a.superset_of(c));

    ASSERT(a.overlaps(b));
    ASSERT(b.overlaps(a));
    ASSERT(!a.disjoint_from(b));

    auto d = real_interval::closed(20, 30);
    ASSERT(a.disjoint_from(d));
    ASSERT(!a.overlaps(d));

    auto e = real_interval::open(10, 20);
    ASSERT(a.adjacent_to(e));
    ASSERT(!a.overlaps(e));
}

TEST(test_interval_operations) {
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(5, 15);

    auto intersection = a & b;
    ASSERT_EQ(intersection, real_interval::closed(5, 10));

    auto hull = a.hull(b);
    ASSERT(hull.has_value());
    ASSERT_EQ(*hull, real_interval::closed(0, 15));

    auto c = real_interval::closed(20, 30);
    auto no_hull = a.hull(c);
    ASSERT(!no_hull.has_value());
}

TEST(test_interval_measures) {
    auto interval = real_interval::closed(2, 8);
    ASSERT_NEAR(interval.length(), 6, 1e-10);
    ASSERT_NEAR(interval.midpoint(), 5, 1e-10);

    auto a = real_interval::closed(0, 10);
    auto b = real_interval::closed(20, 30);
    ASSERT_NEAR(a.distance_to(b), 10, 1e-10);
}

// === Disjoint Interval Set Tests ===

TEST(test_set_construction) {
    auto set1 = real_set{};
    ASSERT(set1.is_empty());

    auto set2 = real_set{real_interval::closed(0, 10)};
    ASSERT(!set2.is_empty());
    ASSERT_EQ(set2.size(), 1);

    auto set3 = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(5, 15),  // Overlaps - will merge
        real_interval::closed(20, 30)
    };
    ASSERT_EQ(set3.size(), 2);  // Should merge first two
}

TEST(test_set_containment) {
    auto set = real_set{}
        .add(0, 10)
        .add(20, 30)
        .add(40, 50);

    ASSERT(set.contains(5));
    ASSERT(set.contains(25));
    ASSERT(!set.contains(15));
    ASSERT(!set.contains(35));

    ASSERT(set.contains(real_interval::closed(2, 8)));
    ASSERT(!set.contains(real_interval::closed(5, 25)));
}

TEST(test_set_operations) {
    auto a = real_set{}.add(0, 10).add(20, 30);
    auto b = real_set{}.add(5, 15).add(25, 35);

    auto union_ab = a | b;
    ASSERT_EQ(union_ab.size(), 2);
    ASSERT(union_ab.contains(real_interval::closed(0, 15)));
    ASSERT(union_ab.contains(real_interval::closed(20, 35)));

    auto intersect_ab = a & b;
    ASSERT_EQ(intersect_ab.size(), 2);
    ASSERT(intersect_ab.contains(real_interval::closed(5, 10)));
    ASSERT(intersect_ab.contains(real_interval::closed(25, 30)));

    auto diff_ab = a - b;
    ASSERT_EQ(diff_ab.size(), 2);
    ASSERT(diff_ab.contains(real_interval::right_open(0, 5)));
    ASSERT(diff_ab.contains(real_interval::right_open(20, 25)));
}

TEST(test_set_queries) {
    auto set = real_set{}.add(10, 20).add(30, 40).add(50, 60);

    auto span = set.span();
    ASSERT_EQ(span, real_interval::closed(10, 60));

    auto gaps = set.gaps();
    ASSERT_EQ(gaps.size(), 2);
    ASSERT(gaps.contains(real_interval::open(20, 30)));
    ASSERT(gaps.contains(real_interval::open(40, 50)));

    ASSERT_EQ(set.component_count(), 3);

    ASSERT_NEAR(set.measure(), 30, 1e-10);
    ASSERT_NEAR(set.gap_measure(), 20, 1e-10);
    ASSERT_NEAR(set.density(), 0.6, 1e-10);
}

TEST(test_fluent_interface) {
    auto set = real_set{}
        .add(0, 10)
        .add(5, 15)  // Will merge with [0, 10]
        .add(20, 30)
        .remove(real_interval::closed(12, 25))
        .add(40, 50);

    ASSERT_EQ(set.size(), 3);
    ASSERT(set.contains(real_interval::closed(0, 11)));
    ASSERT(set.contains(real_interval::closed(26, 30)));
    ASSERT(set.contains(real_interval::closed(40, 50)));
}

TEST(test_functional_operations) {
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
    ASSERT(scaled.contains(2));
    ASSERT(scaled.contains(40));
    ASSERT(scaled.contains(80));
}

// === Parser Tests ===

TEST(test_parser_intervals) {
    auto i1 = parse_interval<double>("[0, 10]");
    ASSERT_EQ(i1, real_interval::closed(0, 10));

    auto i2 = parse_interval<double>("(0, 10)");
    ASSERT_EQ(i2, real_interval::open(0, 10));

    auto i3 = parse_interval<double>("[0, 10)");
    ASSERT_EQ(i3, real_interval::right_open(0, 10));

    auto i4 = parse_interval<double>("{5}");
    ASSERT_EQ(i4, real_interval::point(5));

    auto i5 = parse_interval<double>("{}");
    ASSERT_EQ(i5, real_interval::empty());
}

TEST(test_parser_sets) {
    auto set1 = real_set::from_string("[0,10] U [20,30]");
    ASSERT_EQ(set1.size(), 2);

    auto set2 = real_set::from_string("[0,10) U (20,30] U {50}");
    ASSERT_EQ(set2.size(), 3);

    auto empty = real_set::from_string("{}");
    ASSERT(empty.is_empty());
}

// === Formatter Tests ===

TEST(test_formatter) {
    auto interval = real_interval::closed(0, 10);
    auto formatted = interval_formatter<double>::format(
        interval, interval_formatter<double>::Style::Mathematical);
    ASSERT_EQ(formatted, "[0,10]");

    auto set = real_set{}.add(0, 10).add(20, 30);
    auto set_formatted = interval_formatter<double>::format(
        set, interval_formatter<double>::Style::Mathematical);
    // Should be "[0,10] U [20,30]"
    ASSERT(!set_formatted.empty());
}

// === Edge Cases ===

TEST(test_edge_cases) {
    // Empty operations
    auto empty = real_set{};
    auto non_empty = real_set{real_interval::closed(0, 10)};

    ASSERT_EQ(empty | non_empty, non_empty);
    ASSERT_EQ(empty & non_empty, empty);
    ASSERT(empty.subset_of(non_empty));

    // Single point operations
    auto point_set = real_set::point(5);
    ASSERT(point_set.contains(5));
    ASSERT(!point_set.contains(4.999));

    // Adjacent intervals
    auto a = real_interval::closed(0, 10);
    auto b = real_interval::open(10, 20);
    ASSERT(a.adjacent_to(b));

    auto set_with_adjacent = real_set{a, b};
    ASSERT_EQ(set_with_adjacent.size(), 1);  // Should merge adjacent
}

int main() {
    std::cout << "\n================================\n";
    std::cout << "   Running DIS API Tests\n";
    std::cout << "================================\n\n";

    // Tests are automatically run by their constructors

    std::cout << "\n================================\n";
    std::cout << "Results: " << tests_passed << "/" << tests_run << " tests passed\n";

    if (tests_passed == tests_run) {
        std::cout << "All tests PASSED! ✓\n";
        std::cout << "================================\n";
        return 0;
    } else {
        std::cout << "Some tests FAILED ✗\n";
        std::cout << "================================\n";
        return 1;
    }
}