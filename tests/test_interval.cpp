#include <gtest/gtest.h>
#include <disjoint_interval_set/interval.hpp>
#include <limits>
#include <type_traits>

using namespace disjoint_interval_set;

// Test fixture for interval tests
template <typename T>
class IntervalTest : public ::testing::Test {
protected:
    using value_type = T;
    using interval_type = interval<T>;
};

// Test with different numeric types
using TestTypes = ::testing::Types<int, double, float, long>;
TYPED_TEST_SUITE(IntervalTest, TestTypes);

// Test default constructor creates empty interval
TYPED_TEST(IntervalTest, DefaultConstructorCreatesEmptyInterval) {
    using interval_type = typename TestFixture::interval_type;
    interval_type i;
    EXPECT_TRUE(i.empty());
    EXPECT_TRUE(empty(i));
}

// Test constructor with valid boundaries
TYPED_TEST(IntervalTest, ConstructorWithValidBoundaries) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    // Closed interval [1, 5]
    interval_type closed(T(1), T(5), false, false);
    EXPECT_FALSE(closed.empty());
    EXPECT_EQ(closed.left, T(1));
    EXPECT_EQ(closed.right, T(5));
    EXPECT_FALSE(closed.left_open);
    EXPECT_FALSE(closed.right_open);

    // Open interval (1, 5)
    interval_type open(T(1), T(5), true, true);
    EXPECT_FALSE(open.empty());
    EXPECT_EQ(open.left, T(1));
    EXPECT_EQ(open.right, T(5));
    EXPECT_TRUE(open.left_open);
    EXPECT_TRUE(open.right_open);

    // Half-open interval [1, 5)
    interval_type left_closed(T(1), T(5), false, true);
    EXPECT_FALSE(left_closed.empty());
    EXPECT_FALSE(left_closed.left_open);
    EXPECT_TRUE(left_closed.right_open);

    // Half-open interval (1, 5]
    interval_type right_closed(T(1), T(5), true, false);
    EXPECT_FALSE(right_closed.empty());
    EXPECT_TRUE(right_closed.left_open);
    EXPECT_FALSE(right_closed.right_open);
}

// Test empty interval detection
TYPED_TEST(IntervalTest, EmptyIntervalDetection) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    // Empty when left > right
    interval_type empty1(T(5), T(1));
    EXPECT_TRUE(empty1.empty());

    // Single point closed interval [3,3] is not empty
    interval_type single_point(T(3), T(3), false, false);
    EXPECT_FALSE(single_point.empty());

    // Single point open interval (3,3) is empty
    interval_type empty_open(T(3), T(3), true, true);
    EXPECT_TRUE(empty_open.empty());

    // Half-open single point intervals are empty
    interval_type empty_half1(T(3), T(3), true, false);
    EXPECT_TRUE(empty_half1.empty());

    interval_type empty_half2(T(3), T(3), false, true);
    EXPECT_TRUE(empty_half2.empty());
}

// Test contains method
TYPED_TEST(IntervalTest, ContainsMethod) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    // Closed interval [1, 5]
    interval_type closed(T(1), T(5), false, false);
    EXPECT_TRUE(closed.contains(T(1)));
    EXPECT_TRUE(closed.contains(T(3)));
    EXPECT_TRUE(closed.contains(T(5)));
    EXPECT_FALSE(closed.contains(T(0)));
    EXPECT_FALSE(closed.contains(T(6)));

    // Open interval (1, 5)
    interval_type open(T(1), T(5), true, true);
    EXPECT_FALSE(open.contains(T(1)));
    EXPECT_TRUE(open.contains(T(3)));
    EXPECT_FALSE(open.contains(T(5)));
    EXPECT_FALSE(open.contains(T(0)));
    EXPECT_FALSE(open.contains(T(6)));

    // Half-open interval [1, 5)
    interval_type left_closed(T(1), T(5), false, true);
    EXPECT_TRUE(left_closed.contains(T(1)));
    EXPECT_TRUE(left_closed.contains(T(3)));
    EXPECT_FALSE(left_closed.contains(T(5)));

    // Half-open interval (1, 5]
    interval_type right_closed(T(1), T(5), true, false);
    EXPECT_FALSE(right_closed.contains(T(1)));
    EXPECT_TRUE(right_closed.contains(T(3)));
    EXPECT_TRUE(right_closed.contains(T(5)));

    // Empty interval contains nothing
    interval_type empty_interval;
    EXPECT_FALSE(empty_interval.contains(T(0)));
    EXPECT_FALSE(empty_interval.contains(T(1)));
}

// Test infimum and supremum
TYPED_TEST(IntervalTest, InfimumAndSupremum) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    // Non-empty interval
    interval_type i(T(2), T(8), false, false);
    auto inf = infimum(i);
    auto sup = supremum(i);
    ASSERT_TRUE(inf.has_value());
    ASSERT_TRUE(sup.has_value());
    EXPECT_EQ(inf.value(), T(2));
    EXPECT_EQ(sup.value(), T(8));

    // Empty interval
    interval_type empty_interval;
    auto empty_inf = infimum(empty_interval);
    auto empty_sup = supremum(empty_interval);
    EXPECT_FALSE(empty_inf.has_value());
    EXPECT_FALSE(empty_sup.has_value());
}

// Test subset operator <
TYPED_TEST(IntervalTest, SubsetOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    interval_type i1(T(2), T(5), false, false);  // [2, 5]
    interval_type i2(T(1), T(6), false, false);  // [1, 6]
    interval_type i3(T(3), T(4), false, false);  // [3, 4]
    interval_type empty_interval;

    // i3 ⊆ i1 ⊆ i2
    EXPECT_TRUE(i3 < i1);
    EXPECT_TRUE(i1 < i2);
    EXPECT_TRUE(i3 < i2);

    // Empty set is subset of everything
    EXPECT_TRUE(empty_interval < i1);
    EXPECT_TRUE(empty_interval < empty_interval);

    // Non-empty is not subset of empty
    EXPECT_FALSE(i1 < empty_interval);

    // Test with open/closed boundaries
    interval_type open(T(2), T(5), true, true);     // (2, 5)
    interval_type closed(T(2), T(5), false, false); // [2, 5]
    EXPECT_TRUE(open < closed);  // (2,5) ⊂ [2,5]
    EXPECT_FALSE(closed < open); // [2,5] ⊄ (2,5)

    // Same boundaries but different openness
    interval_type half1(T(2), T(5), false, true);  // [2, 5)
    interval_type half2(T(2), T(5), true, false);  // (2, 5]
    EXPECT_FALSE(half1 < half2);
    EXPECT_FALSE(half2 < half1);
}

// Test equality operator ==
TYPED_TEST(IntervalTest, EqualityOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    interval_type i1(T(2), T(5), false, false);
    interval_type i2(T(2), T(5), false, false);
    interval_type i3(T(2), T(5), true, false);
    interval_type i4(T(3), T(5), false, false);
    interval_type empty1, empty2;

    EXPECT_TRUE(i1 == i2);
    EXPECT_FALSE(i1 == i3);  // Different left_open
    EXPECT_FALSE(i1 == i4);  // Different left value
    EXPECT_TRUE(empty1 == empty2);  // Empty intervals are equal
    EXPECT_FALSE(i1 == empty1);
}

// Test adjacency
TYPED_TEST(IntervalTest, Adjacency) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    // Adjacent intervals [1, 3] and (3, 5]
    interval_type i1(T(1), T(3), false, false);  // [1, 3]
    interval_type i2(T(3), T(5), true, false);   // (3, 5]
    EXPECT_TRUE(adjacent(i1, i2));
    EXPECT_TRUE(adjacent(i2, i1));

    // Adjacent intervals [1, 3) and [3, 5]
    interval_type i3(T(1), T(3), false, true);   // [1, 3)
    interval_type i4(T(3), T(5), false, false);  // [3, 5]
    EXPECT_TRUE(adjacent(i3, i4));
    EXPECT_TRUE(adjacent(i4, i3));

    // Non-adjacent intervals with same boundary but same openness
    interval_type i5(T(1), T(3), false, true);   // [1, 3)
    interval_type i6(T(3), T(5), true, false);   // (3, 5]
    EXPECT_FALSE(adjacent(i5, i6));

    // Non-adjacent intervals (gap between them)
    interval_type i7(T(1), T(3), false, false);  // [1, 3]
    interval_type i8(T(5), T(7), false, false);  // [5, 7]
    EXPECT_FALSE(adjacent(i7, i8));

    // Overlapping intervals are not adjacent
    interval_type i9(T(1), T(4), false, false);   // [1, 4]
    interval_type i10(T(3), T(6), false, false);  // [3, 6]
    EXPECT_FALSE(adjacent(i9, i10));
}

// Test intersection operator *
TYPED_TEST(IntervalTest, IntersectionOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;

    // Overlapping intervals
    interval_type i1(T(1), T(5), false, false);  // [1, 5]
    interval_type i2(T(3), T(7), false, false);  // [3, 7]
    auto intersection = i1 * i2;
    EXPECT_FALSE(intersection.empty());
    EXPECT_EQ(intersection.left, T(3));
    EXPECT_EQ(intersection.right, T(5));
    EXPECT_FALSE(intersection.left_open);
    EXPECT_FALSE(intersection.right_open);

    // Non-overlapping intervals
    interval_type i3(T(1), T(3), false, false);  // [1, 3]
    interval_type i4(T(5), T(7), false, false);  // [5, 7]
    auto empty_intersection = i3 * i4;
    EXPECT_TRUE(empty_intersection.empty());

    // Intersection with empty interval
    interval_type i5(T(1), T(5), false, false);
    interval_type empty_interval;
    auto with_empty = i5 * empty_interval;
    EXPECT_TRUE(with_empty.empty());

    // Test boundary conditions
    interval_type i6(T(1), T(5), false, true);   // [1, 5)
    interval_type i7(T(3), T(7), true, false);   // (3, 7]
    auto intersection2 = i6 * i7;
    EXPECT_FALSE(intersection2.empty());
    EXPECT_EQ(intersection2.left, T(3));
    EXPECT_EQ(intersection2.right, T(5));
    EXPECT_TRUE(intersection2.left_open);   // Inherits from i7
    EXPECT_TRUE(intersection2.right_open);  // Inherits from i6

    // Touching intervals with incompatible boundaries
    interval_type i8(T(1), T(3), false, true);   // [1, 3)
    interval_type i9(T(3), T(5), true, false);   // (3, 5]
    auto no_intersection = i8 * i9;
    EXPECT_TRUE(no_intersection.empty());

    // Touching intervals with compatible boundaries
    interval_type i10(T(1), T(3), false, false);  // [1, 3]
    interval_type i11(T(3), T(5), false, false);  // [3, 5]
    auto point_intersection = i10 * i11;
    EXPECT_FALSE(point_intersection.empty());
    EXPECT_EQ(point_intersection.left, T(3));
    EXPECT_EQ(point_intersection.right, T(3));
    EXPECT_FALSE(point_intersection.left_open);
    EXPECT_FALSE(point_intersection.right_open);
}

// Test lexicographical comparison
TEST(IntervalLexicographicalTest, LexicographicalOrder) {
    using interval_type = interval<int>;

    std::less<interval_type> lex_compare;

    interval_type i1(1, 3, false, false);  // [1, 3]
    interval_type i2(2, 4, false, false);  // [2, 4]
    interval_type i3(1, 5, false, false);  // [1, 5]
    interval_type i4(1, 3, true, false);   // (1, 3]
    interval_type empty1, empty2;

    // Empty intervals come first
    EXPECT_TRUE(lex_compare(empty1, i1));
    EXPECT_FALSE(lex_compare(i1, empty1));
    EXPECT_FALSE(lex_compare(empty1, empty2));  // Equal empty intervals

    // Lower infimum comes first
    EXPECT_TRUE(lex_compare(i1, i2));
    EXPECT_FALSE(lex_compare(i2, i1));

    // Same infimum, closed comes before open
    EXPECT_TRUE(lex_compare(i1, i4));
    EXPECT_FALSE(lex_compare(i4, i1));

    // Same interval
    EXPECT_FALSE(lex_compare(i1, i1));
}

// Test helper functions
TEST(IntervalHelperTest, HelperFunctions) {
    using interval_type = interval<int>;

    interval_type i1(2, 8, true, false);  // (2, 8]

    EXPECT_TRUE(is_left_open(i1));
    EXPECT_FALSE(is_right_open(i1));
    EXPECT_TRUE(contains(i1, 5));
    EXPECT_FALSE(contains(i1, 2));
    EXPECT_TRUE(contains(i1, 8));
}

// Test edge cases with extreme values
TEST(IntervalEdgeCases, ExtremeValues) {
    using interval_type = interval<int>;

    // Maximum range interval
    interval_type max_interval(
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max(),
        false, false
    );
    EXPECT_FALSE(max_interval.empty());
    EXPECT_TRUE(max_interval.contains(0));
    EXPECT_TRUE(max_interval.contains(std::numeric_limits<int>::min()));
    EXPECT_TRUE(max_interval.contains(std::numeric_limits<int>::max()));

    // Test with negative values
    interval_type negative(-10, -5, false, false);
    EXPECT_FALSE(negative.empty());
    EXPECT_TRUE(negative.contains(-7));
    EXPECT_FALSE(negative.contains(-11));
    EXPECT_FALSE(negative.contains(-4));

    // Test crossing zero
    interval_type crossing(-5, 5, false, false);
    EXPECT_TRUE(crossing.contains(0));
    EXPECT_TRUE(crossing.contains(-3));
    EXPECT_TRUE(crossing.contains(3));
}

// Test copy constructor
TEST(IntervalCopyTest, CopyConstructor) {
    using interval_type = interval<double>;

    interval_type original(1.5, 7.5, true, false);
    interval_type copy(original);

    EXPECT_EQ(copy.left, original.left);
    EXPECT_EQ(copy.right, original.right);
    EXPECT_EQ(copy.left_open, original.left_open);
    EXPECT_EQ(copy.right_open, original.right_open);
    EXPECT_TRUE(copy == original);
}

// Test with floating point types
TEST(IntervalFloatingPoint, FloatingPointOperations) {
    using interval_type = interval<double>;

    interval_type i1(1.5, 3.7, false, false);
    interval_type i2(2.3, 4.9, false, false);

    EXPECT_TRUE(i1.contains(2.0));
    EXPECT_TRUE(i1.contains(1.5));
    EXPECT_TRUE(i1.contains(3.7));
    EXPECT_FALSE(i1.contains(1.4));
    EXPECT_FALSE(i1.contains(3.8));

    auto intersection = i1 * i2;
    EXPECT_FALSE(intersection.empty());
    EXPECT_DOUBLE_EQ(intersection.left, 2.3);
    EXPECT_DOUBLE_EQ(intersection.right, 3.7);

    // Test with very small intervals
    interval_type tiny(1.0, 1.0 + std::numeric_limits<double>::epsilon(), false, false);
    EXPECT_FALSE(tiny.empty());
    EXPECT_TRUE(tiny.contains(1.0));
}