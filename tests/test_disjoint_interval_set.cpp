#include <gtest/gtest.h>
#include <disjoint_interval_set/disjoint_interval_set.hpp>
#include <vector>
#include <algorithm>

using namespace disjoint_interval_set;

// Test fixture for disjoint interval set tests
template <typename T>
class DisjointIntervalSetTest : public ::testing::Test {
protected:
    using value_type = T;
    using interval_type = interval<T>;
    using dis_type = disjoint_interval_set<interval_type>;
};

using TestTypes = ::testing::Types<int, double>;
TYPED_TEST_SUITE(DisjointIntervalSetTest, TestTypes);

// Test default constructor creates empty set
TYPED_TEST(DisjointIntervalSetTest, DefaultConstructor) {
    using dis_type = typename TestFixture::dis_type;
    dis_type set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.begin(), set.end());
    EXPECT_FALSE(set.infimum().has_value());
    EXPECT_FALSE(set.supremum().has_value());
}

// Test copy constructor
TYPED_TEST(DisjointIntervalSetTest, CopyConstructor) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type original;
    original = original + dis_type{{interval_type(T(1), T(3), false, false)}};
    original = original + dis_type{{interval_type(T(5), T(7), false, false)}};

    dis_type copy(original);
    EXPECT_EQ(original, copy);
}

// Test contains method
TYPED_TEST(DisjointIntervalSetTest, ContainsMethod) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set;
    set = set + dis_type{{interval_type(T(1), T(3), false, false)}};  // [1,3]
    set = set + dis_type{{interval_type(T(5), T(7), false, false)}};  // [5,7]

    // Points in the intervals
    EXPECT_TRUE(set.contains(T(1)));
    EXPECT_TRUE(set.contains(T(2)));
    EXPECT_TRUE(set.contains(T(3)));
    EXPECT_TRUE(set.contains(T(5)));
    EXPECT_TRUE(set.contains(T(6)));
    EXPECT_TRUE(set.contains(T(7)));

    // Points not in the intervals
    EXPECT_FALSE(set.contains(T(0)));
    EXPECT_FALSE(set.contains(T(4)));
    EXPECT_FALSE(set.contains(T(8)));

    // Empty set contains nothing
    dis_type empty_set;
    EXPECT_FALSE(empty_set.contains(T(0)));
}

// Test infimum and supremum
TYPED_TEST(DisjointIntervalSetTest, InfimumSupremum) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set;
    set = set + dis_type{{interval_type(T(2), T(4), false, false)}};
    set = set + dis_type{{interval_type(T(6), T(10), false, false)}};

    auto inf = set.infimum();
    auto sup = set.supremum();
    ASSERT_TRUE(inf.has_value());
    ASSERT_TRUE(sup.has_value());
    EXPECT_EQ(inf.value(), T(2));
    EXPECT_EQ(sup.value(), T(10));
}

// Test union operator +
TYPED_TEST(DisjointIntervalSetTest, UnionOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    // Union of disjoint intervals
    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(3), false, false)}};

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(5), T(7), false, false)}};

    auto union_set = set1 + set2;
    EXPECT_TRUE(union_set.contains(T(2)));
    EXPECT_TRUE(union_set.contains(T(6)));
    EXPECT_FALSE(union_set.contains(T(4)));

    // Union of overlapping intervals should merge them
    dis_type set3;
    set3 = set3 + dis_type{{interval_type(T(2), T(5), false, false)}};

    auto merged = set1 + set3;  // [1,3] + [2,5] = [1,5]
    EXPECT_TRUE(merged.contains(T(1)));
    EXPECT_TRUE(merged.contains(T(3)));
    EXPECT_TRUE(merged.contains(T(4)));
    EXPECT_TRUE(merged.contains(T(5)));

    // Check that result is actually merged (single interval)
    int interval_count = 0;
    for (auto it = merged.begin(); it != merged.end(); ++it) {
        interval_count++;
    }
    EXPECT_EQ(interval_count, 1);

    // Union with empty set
    dis_type empty_set;
    auto with_empty = set1 + empty_set;
    EXPECT_EQ(with_empty, set1);
}

// Test complement operator ~
TYPED_TEST(DisjointIntervalSetTest, ComplementOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set;
    set = set + dis_type{{interval_type(T(3), T(7), false, false)}};  // [3,7]

    auto complement = ~set;

    // Complement should contain everything except [3,7]
    EXPECT_TRUE(complement.contains(T(2)));
    EXPECT_FALSE(complement.contains(T(3)));
    EXPECT_FALSE(complement.contains(T(5)));
    EXPECT_FALSE(complement.contains(T(7)));
    EXPECT_TRUE(complement.contains(T(8)));

    // Complement of complement should give back original
    auto double_complement = ~(~set);
    EXPECT_EQ(double_complement, set);

    // Complement of empty set should be universal set
    dis_type empty_set;
    auto universal = ~empty_set;
    EXPECT_TRUE(universal.contains(T(0)));
    EXPECT_TRUE(universal.contains(T(100)));
    EXPECT_TRUE(universal.contains(T(-100)));
}

// Test intersection operator *
TYPED_TEST(DisjointIntervalSetTest, IntersectionOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(5), false, false)}};  // [1,5]
    set1 = set1 + dis_type{{interval_type(T(8), T(12), false, false)}}; // [8,12]

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(3), T(7), false, false)}};  // [3,7]
    set2 = set2 + dis_type{{interval_type(T(10), T(15), false, false)}}; // [10,15]

    auto intersection = set1 * set2;

    // Should contain [3,5] and [10,12]
    EXPECT_FALSE(intersection.contains(T(2)));
    EXPECT_TRUE(intersection.contains(T(3)));
    EXPECT_TRUE(intersection.contains(T(4)));
    EXPECT_TRUE(intersection.contains(T(5)));
    EXPECT_FALSE(intersection.contains(T(6)));
    EXPECT_FALSE(intersection.contains(T(9)));
    EXPECT_TRUE(intersection.contains(T(10)));
    EXPECT_TRUE(intersection.contains(T(11)));
    EXPECT_TRUE(intersection.contains(T(12)));
    EXPECT_FALSE(intersection.contains(T(13)));

    // Intersection with empty set
    dis_type empty_set;
    auto with_empty = set1 * empty_set;
    EXPECT_TRUE(with_empty.empty());

    // Intersection with itself
    auto self_intersection = set1 * set1;
    EXPECT_EQ(self_intersection, set1);
}

// Test difference operator -
TYPED_TEST(DisjointIntervalSetTest, DifferenceOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(10), false, false)}};  // [1,10]

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(3), T(5), false, false)}};   // [3,5]
    set2 = set2 + dis_type{{interval_type(T(7), T(9), false, false)}};   // [7,9]

    auto difference = set1 - set2;

    // Should contain [1,3), (5,7), and (9,10]
    EXPECT_TRUE(difference.contains(T(1)));
    EXPECT_TRUE(difference.contains(T(2)));
    EXPECT_FALSE(difference.contains(T(3)));
    EXPECT_FALSE(difference.contains(T(4)));
    EXPECT_FALSE(difference.contains(T(5)));
    EXPECT_TRUE(difference.contains(T(6)));
    EXPECT_FALSE(difference.contains(T(7)));
    EXPECT_FALSE(difference.contains(T(8)));
    EXPECT_FALSE(difference.contains(T(9)));
    EXPECT_TRUE(difference.contains(T(10)));

    // Difference with itself should be empty
    auto self_diff = set1 - set1;
    EXPECT_TRUE(self_diff.empty());

    // Difference with empty set
    dis_type empty_set;
    auto with_empty = set1 - empty_set;
    EXPECT_EQ(with_empty, set1);
}

// Test symmetric difference operator ^
TYPED_TEST(DisjointIntervalSetTest, SymmetricDifferenceOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(5), false, false)}};  // [1,5]

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(3), T(7), false, false)}};  // [3,7]

    auto sym_diff = set1 ^ set2;

    // Should contain [1,3) and (5,7]
    EXPECT_TRUE(sym_diff.contains(T(1)));
    EXPECT_TRUE(sym_diff.contains(T(2)));
    EXPECT_FALSE(sym_diff.contains(T(3)));
    EXPECT_FALSE(sym_diff.contains(T(4)));
    EXPECT_FALSE(sym_diff.contains(T(5)));
    EXPECT_TRUE(sym_diff.contains(T(6)));
    EXPECT_TRUE(sym_diff.contains(T(7)));

    // Symmetric difference with itself should be empty
    auto self_sym_diff = set1 ^ set1;
    EXPECT_TRUE(self_sym_diff.empty());

    // Symmetric difference is commutative
    auto sym_diff2 = set2 ^ set1;
    EXPECT_EQ(sym_diff, sym_diff2);
}

// Test subset operator <=
TYPED_TEST(DisjointIntervalSetTest, SubsetOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(2), T(4), false, false)}};  // [2,4]

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(1), T(5), false, false)}};  // [1,5]

    dis_type set3;
    set3 = set3 + dis_type{{interval_type(T(2), T(3), false, false)}};  // [2,3]

    EXPECT_TRUE(set1 <= set2);   // [2,4] ⊆ [1,5]
    EXPECT_TRUE(set3 <= set1);   // [2,3] ⊆ [2,4]
    EXPECT_TRUE(set3 <= set2);   // [2,3] ⊆ [1,5]
    EXPECT_FALSE(set2 <= set1);  // [1,5] ⊄ [2,4]

    // Empty set is subset of everything
    dis_type empty_set;
    EXPECT_TRUE(empty_set <= set1);
    EXPECT_TRUE(empty_set <= empty_set);

    // Every set is subset of itself
    EXPECT_TRUE(set1 <= set1);
}

// Test superset operator >=
TYPED_TEST(DisjointIntervalSetTest, SupersetOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(5), false, false)}};

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(2), T(4), false, false)}};

    EXPECT_TRUE(set1 >= set2);   // [1,5] ⊇ [2,4]
    EXPECT_FALSE(set2 >= set1);  // [2,4] ⊉ [1,5]
    EXPECT_TRUE(set1 >= set1);   // Self superset
}

// Test equality operator ==
TYPED_TEST(DisjointIntervalSetTest, EqualityOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(3), false, false)}};
    set1 = set1 + dis_type{{interval_type(T(5), T(7), false, false)}};

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(1), T(3), false, false)}};
    set2 = set2 + dis_type{{interval_type(T(5), T(7), false, false)}};

    dis_type set3;
    set3 = set3 + dis_type{{interval_type(T(1), T(7), false, false)}};

    EXPECT_TRUE(set1 == set2);
    EXPECT_FALSE(set1 == set3);
    EXPECT_TRUE(set1 == set1);

    // Empty sets are equal
    dis_type empty1, empty2;
    EXPECT_TRUE(empty1 == empty2);
}

// Test inequality operator !=
TYPED_TEST(DisjointIntervalSetTest, InequalityOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(3), false, false)}};

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(1), T(4), false, false)}};

    EXPECT_TRUE(set1 != set2);
    EXPECT_FALSE(set1 != set1);
}

// Test proper subset operator <
TYPED_TEST(DisjointIntervalSetTest, ProperSubsetOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(2), T(4), false, false)}};

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(1), T(5), false, false)}};

    EXPECT_TRUE(set1 < set2);   // [2,4] ⊂ [1,5]
    EXPECT_FALSE(set2 < set1);  // [1,5] ⊄ [2,4]
    EXPECT_FALSE(set1 < set1);  // Not proper subset of itself
}

// Test proper superset operator >
TYPED_TEST(DisjointIntervalSetTest, ProperSupersetOperator) {
    using T = typename TestFixture::value_type;
    using interval_type = typename TestFixture::interval_type;
    using dis_type = typename TestFixture::dis_type;

    dis_type set1;
    set1 = set1 + dis_type{{interval_type(T(1), T(5), false, false)}};

    dis_type set2;
    set2 = set2 + dis_type{{interval_type(T(2), T(4), false, false)}};

    EXPECT_TRUE(set1 > set2);   // [1,5] ⊃ [2,4]
    EXPECT_FALSE(set2 > set1);  // [2,4] ⊅ [1,5]
    EXPECT_FALSE(set1 > set1);  // Not proper superset of itself
}

// Test with predefined type aliases
TEST(DisjointIntervalSetAliases, RealAndIntegerSets) {
    // Test reals (double)
    reals real_set;
    EXPECT_TRUE(real_set.empty());

    // Test integers (int)
    integers int_set;
    EXPECT_TRUE(int_set.empty());
}

// Test complex operations
TEST(DisjointIntervalSetComplex, ComplexSetOperations) {
    using interval_type = interval<int>;
    using dis_type = disjoint_interval_set<interval_type>;

    // Create multiple sets
    dis_type A;
    A = A + dis_type{{interval_type(1, 3, false, false)}};   // [1,3]
    A = A + dis_type{{interval_type(5, 7, false, false)}};   // [5,7]
    A = A + dis_type{{interval_type(10, 12, false, false)}}; // [10,12]

    dis_type B;
    B = B + dis_type{{interval_type(2, 4, false, false)}};   // [2,4]
    B = B + dis_type{{interval_type(6, 8, false, false)}};   // [6,8]

    dis_type C;
    C = C + dis_type{{interval_type(0, 15, false, false)}};  // [0,15]

    // Test De Morgan's Law: ~(A ∪ B) = ~A ∩ ~B
    auto union_AB = A + B;
    auto complement_union = ~union_AB;
    auto complement_A = ~A;
    auto complement_B = ~B;
    auto intersection_complements = complement_A * complement_B;

    // Check at various points
    for (int i = -5; i <= 20; ++i) {
        EXPECT_EQ(complement_union.contains(i), intersection_complements.contains(i))
            << "De Morgan's Law failed at point " << i;
    }

    // Test distributive law: A ∩ (B ∪ C) = (A ∩ B) ∪ (A ∩ C)
    auto union_BC = B + C;
    auto A_intersect_union = A * union_BC;
    auto A_intersect_B = A * B;
    auto A_intersect_C = A * C;
    auto union_intersections = A_intersect_B + A_intersect_C;

    for (int i = -5; i <= 20; ++i) {
        EXPECT_EQ(A_intersect_union.contains(i), union_intersections.contains(i))
            << "Distributive law failed at point " << i;
    }

    // Test associativity of union: (A ∪ B) ∪ C = A ∪ (B ∪ C)
    auto AB_then_C = (A + B) + C;
    auto A_then_BC = A + (B + C);

    for (int i = -5; i <= 20; ++i) {
        EXPECT_EQ(AB_then_C.contains(i), A_then_BC.contains(i))
            << "Associativity of union failed at point " << i;
    }
}

// Test iterator interface
TEST(DisjointIntervalSetIterator, IteratorOperations) {
    using interval_type = interval<int>;
    using dis_type = disjoint_interval_set<interval_type>;

    dis_type set;
    set = set + dis_type{{interval_type(1, 3, false, false)}};
    set = set + dis_type{{interval_type(5, 7, false, false)}};
    set = set + dis_type{{interval_type(10, 12, false, false)}};

    // Count intervals using iterator
    int count = 0;
    for (auto it = set.begin(); it != set.end(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 3);

    // Check intervals are in order
    auto it = set.begin();
    EXPECT_EQ(it->left, 1);
    EXPECT_EQ(it->right, 3);
    ++it;
    EXPECT_EQ(it->left, 5);
    EXPECT_EQ(it->right, 7);
    ++it;
    EXPECT_EQ(it->left, 10);
    EXPECT_EQ(it->right, 12);
    ++it;
    EXPECT_EQ(it, set.end());

    // Range-based for loop
    std::vector<int> left_bounds;
    for (const auto& interval : set) {
        left_bounds.push_back(interval.left);
    }
    EXPECT_EQ(left_bounds.size(), 3u);
    EXPECT_EQ(left_bounds[0], 1);
    EXPECT_EQ(left_bounds[1], 5);
    EXPECT_EQ(left_bounds[2], 10);
}

// Test edge cases
TEST(DisjointIntervalSetEdgeCases, EdgeCaseHandling) {
    using interval_type = interval<int>;
    using dis_type = disjoint_interval_set<interval_type>;

    // Test with adjacent intervals that should merge
    dis_type adjacent;
    adjacent = adjacent + dis_type{{interval_type(1, 3, false, false)}};  // [1,3]
    adjacent = adjacent + dis_type{{interval_type(3, 5, false, false)}};  // [3,5]

    // Should merge to single interval [1,5]
    int interval_count = 0;
    for (auto it = adjacent.begin(); it != adjacent.end(); ++it) {
        interval_count++;
        EXPECT_EQ(it->left, 1);
        EXPECT_EQ(it->right, 5);
    }
    EXPECT_EQ(interval_count, 1);

    // Test with open/closed boundary merging
    dis_type boundary_test;
    boundary_test = boundary_test + dis_type{{interval_type(1, 3, false, true)}};  // [1,3)
    boundary_test = boundary_test + dis_type{{interval_type(3, 5, false, false)}}; // [3,5]

    // Should merge to [1,5]
    interval_count = 0;
    for (auto it = boundary_test.begin(); it != boundary_test.end(); ++it) {
        interval_count++;
    }
    EXPECT_EQ(interval_count, 1);
    EXPECT_TRUE(boundary_test.contains(3)); // The point 3 should be included
}