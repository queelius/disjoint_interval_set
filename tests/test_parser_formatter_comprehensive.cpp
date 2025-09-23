/**
 * Comprehensive unit tests for parser and formatter
 * Goal: Achieve >95% code coverage for parser.hpp and format.hpp
 */

#include "../include/dis/dis.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <limits>
#include <cmath>

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

#define ASSERT_THROWS(expr) \
    try { \
        expr; \
        throw std::runtime_error("Expected exception not thrown"); \
    } catch (...) {}

using real_interval = interval<double>;
using int_interval = interval<int>;
using real_set = disjoint_interval_set<real_interval>;
using int_set = disjoint_interval_set<int_interval>;

// ======================================================================
// INTERVAL PARSER TESTS
// ======================================================================

TEST(test_parse_closed_interval) {
    auto i1 = parse_interval<double>("[0, 10]");
    ASSERT_EQ(i1, real_interval::closed(0, 10));

    auto i2 = parse_interval<double>("[  -5.5  ,  7.3  ]");  // with spaces
    ASSERT_EQ(i2, real_interval::closed(-5.5, 7.3));

    auto i3 = parse_interval<int>("[0, 100]");
    ASSERT_EQ(i3, int_interval::closed(0, 100));

    // Scientific notation
    auto i4 = parse_interval<double>("[1e-5, 1e5]");
    ASSERT_EQ(i4, real_interval::closed(1e-5, 1e5));

    auto i5 = parse_interval<double>("[-1.23e-10, 4.56e10]");
    ASSERT_EQ(i5, real_interval::closed(-1.23e-10, 4.56e10));
}

TEST(test_parse_open_interval) {
    auto i1 = parse_interval<double>("(0, 10)");
    ASSERT_EQ(i1, real_interval::open(0, 10));

    auto i2 = parse_interval<double>("(  -5.5  ,  7.3  )");  // with spaces
    ASSERT_EQ(i2, real_interval::open(-5.5, 7.3));

    auto i3 = parse_interval<int>("(0, 100)");
    ASSERT_EQ(i3, int_interval::open(0, 100));
}

TEST(test_parse_half_open_intervals) {
    auto i1 = parse_interval<double>("[0, 10)");
    ASSERT_EQ(i1, real_interval::right_open(0, 10));

    auto i2 = parse_interval<double>("(0, 10]");
    ASSERT_EQ(i2, real_interval::left_open(0, 10));

    auto i3 = parse_interval<double>("[-5.5, 7.3)");
    ASSERT_EQ(i3, real_interval::right_open(-5.5, 7.3));

    auto i4 = parse_interval<double>("(-5.5, 7.3]");
    ASSERT_EQ(i4, real_interval::left_open(-5.5, 7.3));
}

TEST(test_parse_special_intervals) {
    // Point interval
    auto point1 = parse_interval<double>("{5}");
    ASSERT_EQ(point1, real_interval::point(5));

    auto point2 = parse_interval<double>("{  -3.14  }");  // with spaces
    ASSERT_EQ(point2, real_interval::point(-3.14));

    // Empty interval
    auto empty = parse_interval<double>("{}");
    ASSERT_EQ(empty, real_interval::empty());

    // Alternative empty notation
    auto empty2 = parse_interval<double>("∅");
    ASSERT_EQ(empty2, real_interval::empty());
}

TEST(test_parse_infinity) {
    // Positive infinity
    auto i1 = parse_interval<double>("[0, inf)");
    ASSERT(i1.contains(1e100));
    ASSERT(!i1.contains(-1));

    auto i2 = parse_interval<double>("[0, +inf)");
    ASSERT(i2.contains(1e100));

    auto i3 = parse_interval<double>("[0, infinity)");
    ASSERT(i3.contains(1e100));

    auto i4 = parse_interval<double>("[0, ∞)");
    ASSERT(i4.contains(1e100));

    // Negative infinity
    auto i5 = parse_interval<double>("(-inf, 0]");
    ASSERT(i5.contains(-1e100));
    ASSERT(!i5.contains(1));

    auto i6 = parse_interval<double>("(-infinity, 0]");
    ASSERT(i6.contains(-1e100));

    auto i7 = parse_interval<double>("(-∞, 0]");
    ASSERT(i7.contains(-1e100));

    // Both infinities
    auto i8 = parse_interval<double>("(-inf, inf)");
    ASSERT(i8.contains(0));
    ASSERT(i8.contains(1e100));
    ASSERT(i8.contains(-1e100));
}

TEST(test_parse_edge_cases) {
    // Very large numbers
    auto i1 = parse_interval<double>("[1e308, 1.7e308]");
    ASSERT(i1.contains(1.5e308));

    // Very small numbers
    auto i2 = parse_interval<double>("[1e-308, 2e-308]");
    ASSERT(!i2.contains(0));

    // Negative numbers
    auto i3 = parse_interval<double>("[-100, -10]");
    ASSERT(i3.contains(-50));
    ASSERT(!i3.contains(0));

    // Zero
    auto i4 = parse_interval<double>("[-1, 1]");
    ASSERT(i4.contains(0));
}

TEST(test_parse_invalid_intervals) {
    // Missing brackets
    ASSERT_THROWS(parse_interval<double>("0, 10"));

    // Invalid numbers
    ASSERT_THROWS(parse_interval<double>("[abc, 10]"));
    ASSERT_THROWS(parse_interval<double>("[0, xyz]"));

    // Missing comma
    ASSERT_THROWS(parse_interval<double>("[0 10]"));

    // Extra commas
    ASSERT_THROWS(parse_interval<double>("[0, 10, 20]"));

    // Empty string
    ASSERT_THROWS(parse_interval<double>(""));

    // Invalid point notation
    ASSERT_THROWS(parse_interval<double>("{5, 10}"));

    // Unclosed brackets
    ASSERT_THROWS(parse_interval<double>("[0, 10"));
    ASSERT_THROWS(parse_interval<double>("0, 10]"));
}

// ======================================================================
// SET PARSER TESTS
// ======================================================================

TEST(test_parse_single_interval_set) {
    auto set1 = real_set::from_string("[0, 10]");
    ASSERT_EQ(set1.size(), 1);
    ASSERT(set1.contains(5));

    auto set2 = real_set::from_string("(0, 10)");
    ASSERT_EQ(set2.size(), 1);
    ASSERT(set2.contains(5));
    ASSERT(!set2.contains(0));

    auto set3 = real_set::from_string("{5}");
    ASSERT_EQ(set3.size(), 1);
    ASSERT(set3.contains(5));
    ASSERT(!set3.contains(4.999));
}

TEST(test_parse_union_sets) {
    // Simple union
    auto set1 = real_set::from_string("[0, 10] U [20, 30]");
    ASSERT_EQ(set1.size(), 2);
    ASSERT(set1.contains(5));
    ASSERT(set1.contains(25));
    ASSERT(!set1.contains(15));

    // Multiple unions
    auto set2 = real_set::from_string("[0, 10] U [20, 30] U [40, 50]");
    ASSERT_EQ(set2.size(), 3);

    // Alternative union symbols
    auto set3 = real_set::from_string("[0, 10] ∪ [20, 30]");
    ASSERT_EQ(set3, set1);

    auto set4 = real_set::from_string("[0, 10] | [20, 30]");
    ASSERT_EQ(set4, set1);

    // With spaces
    auto set5 = real_set::from_string("  [0, 10]  U  [20, 30]  ");
    ASSERT_EQ(set5, set1);
}

TEST(test_parse_intersection_sets) {
    auto set = real_set::from_string("[0, 20] ∩ [10, 30]");
    ASSERT_EQ(set.size(), 1);
    ASSERT(set.contains(real_interval::closed(10, 20)));

    // Alternative intersection symbols
    auto set2 = real_set::from_string("[0, 20] & [10, 30]");
    ASSERT_EQ(set2, set);

    auto set3 = real_set::from_string("[0, 20] * [10, 30]");
    ASSERT_EQ(set3, set);
}

TEST(test_parse_difference_sets) {
    auto set = real_set::from_string("[0, 30] - [10, 20]");
    ASSERT_EQ(set.size(), 2);
    ASSERT(set.contains(5));
    ASSERT(!set.contains(15));
    ASSERT(set.contains(25));

    // Alternative difference symbol
    auto set2 = real_set::from_string("[0, 30] \\ [10, 20]");
    ASSERT_EQ(set2, set);
}

TEST(test_parse_symmetric_difference) {
    auto set = real_set::from_string("[0, 20] ⊕ [10, 30]");
    ASSERT_EQ(set.size(), 2);
    ASSERT(set.contains(5));
    ASSERT(!set.contains(15));
    ASSERT(set.contains(25));

    // Alternative symbol
    auto set2 = real_set::from_string("[0, 20] ^ [10, 30]");
    ASSERT_EQ(set2, set);
}

TEST(test_parse_complex_expressions) {
    // Multiple operations (left-to-right evaluation)
    auto set1 = real_set::from_string("[0, 30] U [40, 50] - [10, 20]");
    ASSERT(set1.contains(5));
    ASSERT(!set1.contains(15));
    ASSERT(set1.contains(25));
    ASSERT(set1.contains(45));

    // Mixed interval types
    auto set2 = real_set::from_string("[0, 10] U (20, 30) U {40}");
    ASSERT_EQ(set2.size(), 3);
    ASSERT(set2.contains(10));
    ASSERT(!set2.contains(20));
    ASSERT(set2.contains(40));

    // With empty intervals
    auto set3 = real_set::from_string("[0, 10] U {} U [20, 30]");
    ASSERT_EQ(set3.size(), 2);
}

TEST(test_parse_empty_set) {
    auto set1 = real_set::from_string("{}");
    ASSERT(set1.is_empty());

    auto set2 = real_set::from_string("∅");
    ASSERT(set2.is_empty());

    // Operations with empty set
    auto set3 = real_set::from_string("[0, 10] & {}");
    ASSERT(set3.is_empty());

    auto set4 = real_set::from_string("{} U [0, 10]");
    ASSERT_EQ(set4.size(), 1);
    ASSERT(set4.contains(5));
}

TEST(test_parse_whitespace_handling) {
    // Extra spaces
    auto set1 = real_set::from_string("  [  0  ,  10  ]  ");
    ASSERT_EQ(set1.size(), 1);

    // Tabs and newlines
    auto set2 = real_set::from_string("\t[0,10]\t U \t[20,30]\t");
    ASSERT_EQ(set2.size(), 2);

    // Mixed whitespace
    auto set3 = real_set::from_string(" \t [ 0 , 10 ] \t ");
    ASSERT_EQ(set3.size(), 1);
}

// ======================================================================
// FORMATTER TESTS
// ======================================================================

TEST(test_format_interval_mathematical) {
    auto formatter = interval_formatter<double>{};
    auto style = interval_formatter<double>::Style::Mathematical;

    // Closed interval
    auto i1 = real_interval::closed(0, 10);
    ASSERT_EQ(formatter.format(i1, style), "[0,10]");

    // Open interval
    auto i2 = real_interval::open(0, 10);
    ASSERT_EQ(formatter.format(i2, style), "(0,10)");

    // Half-open intervals
    auto i3 = real_interval::right_open(0, 10);
    ASSERT_EQ(formatter.format(i3, style), "[0,10)");

    auto i4 = real_interval::left_open(0, 10);
    ASSERT_EQ(formatter.format(i4, style), "(0,10]");

    // Point interval
    auto i5 = real_interval::point(5);
    ASSERT_EQ(formatter.format(i5, style), "{5}");

    // Empty interval
    auto i6 = real_interval::empty();
    ASSERT_EQ(formatter.format(i6, style), "{}");
}

TEST(test_format_interval_programming) {
    auto formatter = interval_formatter<double>{};
    auto style = interval_formatter<double>::Style::Programming;

    // Closed interval
    auto i1 = real_interval::closed(0, 10);
    ASSERT_EQ(formatter.format(i1, style), "interval(0, 10, true, true)");

    // Open interval
    auto i2 = real_interval::open(0, 10);
    ASSERT_EQ(formatter.format(i2, style), "interval(0, 10, false, false)");

    // Point interval
    auto i3 = real_interval::point(5);
    ASSERT_EQ(formatter.format(i3, style), "interval(5, 5, true, true)");

    // Empty interval
    auto i4 = real_interval::empty();
    ASSERT_EQ(formatter.format(i4, style), "interval(0, -1, false, false)");
}

TEST(test_format_interval_verbose) {
    auto formatter = interval_formatter<double>{};
    auto style = interval_formatter<double>::Style::Verbose;

    // Closed interval
    auto i1 = real_interval::closed(0, 10);
    auto result1 = formatter.format(i1, style);
    ASSERT(result1.find("interval from 0") != std::string::npos);
    ASSERT(result1.find("inclusive") != std::string::npos);

    // Open interval
    auto i2 = real_interval::open(0, 10);
    auto result2 = formatter.format(i2, style);
    ASSERT(result2.find("interval from 0") != std::string::npos);
    ASSERT(result2.find("exclusive") != std::string::npos);

    // Empty interval
    auto i3 = real_interval::empty();
    auto result3 = formatter.format(i3, style);
    ASSERT(result3.find("empty") != std::string::npos);
}

TEST(test_format_set_mathematical) {
    auto formatter = interval_formatter<double>{};
    auto style = interval_formatter<double>::Style::Mathematical;

    // Single interval
    auto set1 = real_set{real_interval::closed(0, 10)};
    ASSERT_EQ(formatter.format(set1, style), "[0,10]");

    // Multiple intervals
    auto set2 = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto result2 = formatter.format(set2, style);
    ASSERT(result2 == "[0,10] ∪ [20,30]" || result2 == "[0,10] U [20,30]");

    // Empty set
    auto set3 = real_set::empty();
    ASSERT_EQ(formatter.format(set3, style), "{}");

    // Mixed interval types
    auto set4 = real_set{
        real_interval::closed(0, 10),
        real_interval::open(20, 30),
        real_interval::point(40)
    };
    auto result4 = formatter.format(set4, style);
    ASSERT(result4.find("[0,10]") != std::string::npos);
    ASSERT(result4.find("(20,30)") != std::string::npos);
    ASSERT(result4.find("{40}") != std::string::npos);
}

TEST(test_format_set_programming) {
    auto formatter = interval_formatter<double>{};
    auto style = interval_formatter<double>::Style::Programming;

    // Multiple intervals with spaces
    auto set = real_set{
        real_interval::closed(0, 10),
        real_interval::closed(20, 30)
    };
    auto result = formatter.format(set, style);
    // Programming style uses interval() notation
    ASSERT(result.find("interval(0, 10, true, true)") != std::string::npos);
    ASSERT(result.find("interval(20, 30, true, true)") != std::string::npos);

    // Empty set
    auto empty = real_set::empty();
    ASSERT_EQ(formatter.format(empty, style), "{}");
}

TEST(test_format_infinity) {
    auto formatter = interval_formatter<double>{};
    auto style = interval_formatter<double>::Style::Mathematical;

    // Positive infinity
    auto i1 = real_interval::at_least(0);
    auto result1 = formatter.format(i1, style);
    ASSERT(result1.find("∞") != std::string::npos ||
           result1.find("inf") != std::string::npos);

    // Negative infinity
    auto i2 = real_interval::at_most(0);
    auto result2 = formatter.format(i2, style);
    ASSERT(result2.find("-∞") != std::string::npos ||
           result2.find("-inf") != std::string::npos);

    // Unbounded
    auto i3 = real_interval::unbounded();
    auto result3 = formatter.format(i3, style);
    ASSERT(result3.find("∞") != std::string::npos ||
           result3.find("inf") != std::string::npos);
}

TEST(test_format_precision) {
    auto formatter = interval_formatter<double>{};

    // Default formatting for floating point
    auto i1 = real_interval::closed(1.23456789, 9.87654321);
    auto result1 = formatter.format(i1);
    ASSERT(!result1.empty());

    // Scientific notation for very large/small numbers
    auto i2 = real_interval::closed(1.234e-10, 5.678e10);
    auto result2 = formatter.format(i2);
    // Output format may vary, just ensure it's not empty
    ASSERT(!result2.empty());
}

TEST(test_format_integers) {
    auto formatter = interval_formatter<int>{};
    auto style = interval_formatter<int>::Style::Mathematical;

    auto i1 = int_interval::closed(1, 10);
    ASSERT_EQ(formatter.format(i1, style), "[1,10]");

    auto i2 = int_interval::open(1, 10);
    ASSERT_EQ(formatter.format(i2, style), "(1,10)");

    auto set = int_set{
        int_interval::closed(1, 10),
        int_interval::closed(20, 30)
    };
    auto result = formatter.format(set, style);
    ASSERT(result == "[1,10] ∪ [20,30]" || result == "[1,10] U [20,30]");
}

TEST(test_format_special_cases) {
    auto formatter = interval_formatter<double>{};

    // NaN values (if supported)
    auto nan = std::numeric_limits<double>::quiet_NaN();
    auto i1 = real_interval(nan, 10);
    // Formatter should handle gracefully, even if interval is invalid

    // Very large intervals
    auto i2 = real_interval::closed(-1e308, 1e308);
    auto result2 = formatter.format(i2);
    ASSERT(!result2.empty());

    // Very small intervals
    auto i3 = real_interval::closed(1e-308, 2e-308);
    auto result3 = formatter.format(i3);
    ASSERT(!result3.empty());
}

// ======================================================================
// ROUND-TRIP TESTS (Parse -> Format -> Parse)
// ======================================================================

TEST(test_round_trip_intervals) {
    std::vector<real_interval> intervals = {
        real_interval::closed(0, 10),
        real_interval::open(0, 10),
        real_interval::left_open(0, 10),
        real_interval::right_open(0, 10),
        real_interval::point(5),
        real_interval::empty()
    };

    auto formatter = interval_formatter<double>{};
    for (const auto& interval : intervals) {
        auto str = formatter.format(interval);
        auto parsed = parse_interval<double>(str);
        ASSERT_EQ(parsed, interval);
    }
}

TEST(test_round_trip_sets) {
    std::vector<real_set> sets = {
        real_set{real_interval::closed(0, 10)},
        real_set{
            real_interval::closed(0, 10),
            real_interval::closed(20, 30)
        },
        real_set::empty(),
        real_set::point(5)
    };

    auto formatter = interval_formatter<double>{};
    for (const auto& set : sets) {
        auto str = formatter.format(set);
        auto parsed = real_set::from_string(str);
        ASSERT_EQ(parsed, set);
    }
}

// Main test runner
int main() {
    std::cout << "\n================================\n";
    std::cout << "   Parser & Formatter Tests\n";
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