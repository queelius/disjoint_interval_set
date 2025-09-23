#include <gtest/gtest.h>
#include <disjoint_interval_set/disjoint_interval_set_algorithms.hpp>
#include <disjoint_interval_set/interval.hpp>
#include <vector>
#include <limits>

using namespace disjoint_interval_set;

// Test merge_overlapping_intervals / make_disjoint_interval_set
TEST(AlgorithmsTest, MergeOverlappingIntervals) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    // Test merging overlapping intervals
    vector_type overlapping = {
        interval_type(1, 3, false, false),   // [1,3]
        interval_type(2, 5, false, false),   // [2,5]
        interval_type(7, 10, false, false),  // [7,10]
        interval_type(8, 12, false, false)   // [8,12]
    };

    auto merged = merge_overlapping_intervals(overlapping);

    // Should result in [1,5] and [7,12]
    ASSERT_EQ(merged.size(), 2u);
    EXPECT_EQ(merged[0].left, 1);
    EXPECT_EQ(merged[0].right, 5);
    EXPECT_FALSE(merged[0].left_open);
    EXPECT_FALSE(merged[0].right_open);
    EXPECT_EQ(merged[1].left, 7);
    EXPECT_EQ(merged[1].right, 12);

    // Test with non-overlapping intervals
    vector_type non_overlapping = {
        interval_type(1, 2, false, false),   // [1,2]
        interval_type(4, 5, false, false),   // [4,5]
        interval_type(7, 8, false, false)    // [7,8]
    };

    auto not_merged = merge_overlapping_intervals(non_overlapping);
    EXPECT_EQ(not_merged.size(), 3u);

    // Test with touching intervals
    vector_type touching = {
        interval_type(1, 3, false, false),   // [1,3]
        interval_type(3, 5, false, false),   // [3,5]
        interval_type(5, 7, false, false)    // [5,7]
    };

    auto merged_touching = merge_overlapping_intervals(touching);

    // Should merge to single interval [1,7]
    ASSERT_EQ(merged_touching.size(), 1u);
    EXPECT_EQ(merged_touching[0].left, 1);
    EXPECT_EQ(merged_touching[0].right, 7);

    // Test with unsorted intervals
    vector_type unsorted = {
        interval_type(5, 7, false, false),   // [5,7]
        interval_type(1, 3, false, false),   // [1,3]
        interval_type(2, 4, false, false)    // [2,4]
    };

    auto merged_unsorted = merge_overlapping_intervals(unsorted);

    // Should merge to [1,4] and [5,7]
    ASSERT_EQ(merged_unsorted.size(), 2u);
    EXPECT_EQ(merged_unsorted[0].left, 1);
    EXPECT_EQ(merged_unsorted[0].right, 4);
    EXPECT_EQ(merged_unsorted[1].left, 5);
    EXPECT_EQ(merged_unsorted[1].right, 7);

    // Test with single interval
    vector_type single = {interval_type(1, 5, false, false)};
    auto merged_single = merge_overlapping_intervals(single);
    ASSERT_EQ(merged_single.size(), 1u);
    EXPECT_EQ(merged_single[0].left, 1);
    EXPECT_EQ(merged_single[0].right, 5);

    // Test with empty vector
    vector_type empty;
    auto merged_empty = merge_overlapping_intervals(empty);
    EXPECT_TRUE(merged_empty.empty());
}

// Test make_disjoint_interval_set specifically
TEST(AlgorithmsTest, MakeDisjointIntervalSet) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    // Test with intervals that have gaps between them
    vector_type intervals = {
        interval_type(10, 15, false, false),  // [10,15]
        interval_type(1, 5, false, false),    // [1,5]
        interval_type(3, 8, false, false),    // [3,8]
        interval_type(20, 25, false, false)   // [20,25]
    };

    auto disjoint = make_disjoint_interval_set(intervals);

    // Should result in [1,8], [10,15], [20,25]
    ASSERT_EQ(disjoint.size(), 3u);
    EXPECT_EQ(disjoint[0].left, 1);
    EXPECT_EQ(disjoint[0].right, 8);
    EXPECT_EQ(disjoint[1].left, 10);
    EXPECT_EQ(disjoint[1].right, 15);
    EXPECT_EQ(disjoint[2].left, 20);
    EXPECT_EQ(disjoint[2].right, 25);

    // Test with fully overlapping intervals
    vector_type fully_overlapping = {
        interval_type(1, 10, false, false),   // [1,10]
        interval_type(2, 8, false, false),    // [2,8]
        interval_type(3, 7, false, false)     // [3,7]
    };

    auto merged_full = make_disjoint_interval_set(fully_overlapping);

    // Should result in single interval [1,10]
    ASSERT_EQ(merged_full.size(), 1u);
    EXPECT_EQ(merged_full[0].left, 1);
    EXPECT_EQ(merged_full[0].right, 10);
}

// Test union_disjoint_interval_sets
TEST(AlgorithmsTest, UnionDisjointIntervalSets) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    vector_type set1 = {
        interval_type(1, 3, false, false),   // [1,3]
        interval_type(5, 7, false, false)    // [5,7]
    };

    vector_type set2 = {
        interval_type(2, 4, false, false),   // [2,4]
        interval_type(8, 10, false, false)   // [8,10]
    };

    auto union_result = union_disjoint_interval_sets(set1, set2);

    // Should result in [1,4], [5,7], [8,10]
    ASSERT_EQ(union_result.size(), 3u);
    EXPECT_EQ(union_result[0].left, 1);
    EXPECT_EQ(union_result[0].right, 4);
    EXPECT_EQ(union_result[1].left, 5);
    EXPECT_EQ(union_result[1].right, 7);
    EXPECT_EQ(union_result[2].left, 8);
    EXPECT_EQ(union_result[2].right, 10);

    // Test with empty sets
    vector_type empty1, empty2;
    auto union_empty = union_disjoint_interval_sets(empty1, empty2);
    EXPECT_TRUE(union_empty.empty());

    auto union_with_empty = union_disjoint_interval_sets(set1, empty1);
    EXPECT_EQ(union_with_empty.size(), set1.size());

    auto union_empty_with = union_disjoint_interval_sets(empty1, set1);
    EXPECT_EQ(union_empty_with.size(), set1.size());
}

// Test complement_disjoint_interval_set with default bounds
TEST(AlgorithmsTest, ComplementDisjointIntervalSetDefaultBounds) {
    using interval_type = interval<double>;
    using vector_type = std::vector<interval_type>;

    vector_type set = {
        interval_type(1.0, 3.0, false, false),   // [1,3]
        interval_type(5.0, 7.0, false, false)    // [5,7]
    };

    auto complement = complement_disjoint_interval_set(set);

    // Should result in (-inf,1), (3,5), (7,inf)
    ASSERT_EQ(complement.size(), 3u);

    // First interval: (-inf, 1)
    EXPECT_EQ(complement[0].left, -std::numeric_limits<double>::infinity());
    EXPECT_EQ(complement[0].right, 1.0);
    EXPECT_FALSE(complement[0].left_open);
    EXPECT_FALSE(complement[0].right_open);

    // Second interval: (3, 5)
    EXPECT_EQ(complement[1].left, 3.0);
    EXPECT_EQ(complement[1].right, 5.0);
    EXPECT_FALSE(complement[1].left_open);
    EXPECT_FALSE(complement[1].right_open);

    // Third interval: (7, inf)
    EXPECT_EQ(complement[2].left, 7.0);
    EXPECT_EQ(complement[2].right, std::numeric_limits<double>::infinity());
    EXPECT_FALSE(complement[2].left_open);
    EXPECT_FALSE(complement[2].right_open);
}

// Test complement_disjoint_interval_set with custom bounds
TEST(AlgorithmsTest, ComplementDisjointIntervalSetCustomBounds) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    vector_type set = {
        interval_type(3, 5, false, false),   // [3,5]
        interval_type(8, 10, false, false)   // [8,10]
    };

    // Custom bounds from 0 to 12
    auto complement = complement_disjoint_interval_set(set, 0, 12);

    // Should result in [0,3), (5,8), (10,12]
    ASSERT_EQ(complement.size(), 3u);

    // First interval: [0, 3)
    EXPECT_EQ(complement[0].left, 0);
    EXPECT_EQ(complement[0].right, 3);

    // Second interval: (5, 8)
    EXPECT_EQ(complement[1].left, 5);
    EXPECT_EQ(complement[1].right, 8);

    // Third interval: (10, 12]
    EXPECT_EQ(complement[2].left, 10);
    EXPECT_EQ(complement[2].right, 12);

    // Test with empty set
    vector_type empty_set;
    auto complement_empty = complement_disjoint_interval_set(empty_set, 0, 10);

    // Should result in single interval [0,10]
    ASSERT_EQ(complement_empty.size(), 1u);
    EXPECT_EQ(complement_empty[0].left, 0);
    EXPECT_EQ(complement_empty[0].right, 10);

    // Test when intervals cover entire range
    vector_type full_coverage = {
        interval_type(0, 10, false, false)   // [0,10]
    };
    auto complement_full = complement_disjoint_interval_set(full_coverage, 0, 10);
    EXPECT_TRUE(complement_full.empty());

    // Test when intervals extend beyond bounds
    vector_type beyond_bounds = {
        interval_type(5, 15, false, false)   // [5,15]
    };
    auto complement_beyond = complement_disjoint_interval_set(beyond_bounds, 0, 10);

    // Should result in [0,5)
    ASSERT_EQ(complement_beyond.size(), 1u);
    EXPECT_EQ(complement_beyond[0].left, 0);
    EXPECT_EQ(complement_beyond[0].right, 5);
}

// Test complement with unsorted input
TEST(AlgorithmsTest, ComplementUnsortedInput) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    // Unsorted intervals
    vector_type unsorted = {
        interval_type(8, 10, false, false),  // [8,10]
        interval_type(3, 5, false, false),   // [3,5]
        interval_type(1, 2, false, false)    // [1,2]
    };

    auto complement = complement_disjoint_interval_set(unsorted, 0, 12);

    // Should handle unsorted input correctly
    // Result should be [0,1), (2,3), (5,8), (10,12]
    ASSERT_EQ(complement.size(), 4u);
    EXPECT_EQ(complement[0].left, 0);
    EXPECT_EQ(complement[0].right, 1);
    EXPECT_EQ(complement[1].left, 2);
    EXPECT_EQ(complement[1].right, 3);
    EXPECT_EQ(complement[2].left, 5);
    EXPECT_EQ(complement[2].right, 8);
    EXPECT_EQ(complement[3].left, 10);
    EXPECT_EQ(complement[3].right, 12);
}

// Test with different interval types (open/closed)
TEST(AlgorithmsTest, MixedOpenClosedIntervals) {
    using interval_type = interval<double>;
    using vector_type = std::vector<interval_type>;

    vector_type mixed = {
        interval_type(1.0, 3.0, true, false),   // (1,3]
        interval_type(3.0, 5.0, false, true),   // [3,5)
        interval_type(7.0, 9.0, true, true)     // (7,9)
    };

    auto merged = make_disjoint_interval_set(mixed);

    // (1,3] and [3,5) should merge to (1,5)
    // (7,9) remains separate
    ASSERT_EQ(merged.size(), 2u);
    EXPECT_EQ(merged[0].left, 1.0);
    EXPECT_EQ(merged[0].right, 5.0);
    EXPECT_FALSE(merged[0].left_open);
    EXPECT_FALSE(merged[0].right_open);
    EXPECT_EQ(merged[1].left, 7.0);
    EXPECT_EQ(merged[1].right, 9.0);
    EXPECT_FALSE(merged[1].left_open);
    EXPECT_FALSE(merged[1].right_open);
}

// Test edge cases with empty intervals
TEST(AlgorithmsTest, EmptyIntervalHandling) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    // Include some empty intervals (where left > right)
    vector_type with_empty = {
        interval_type(1, 3, false, false),    // [1,3] - valid
        interval_type(5, 2, false, false),    // Empty (5 > 2)
        interval_type(7, 9, false, false),    // [7,9] - valid
        interval_type(10, 8, false, false)    // Empty (10 > 8)
    };

    auto result = make_disjoint_interval_set(with_empty);

    // Should only include non-empty intervals
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].left, 1);
    EXPECT_EQ(result[0].right, 3);
    EXPECT_EQ(result[1].left, 7);
    EXPECT_EQ(result[1].right, 9);
}

// Test with single-point intervals
TEST(AlgorithmsTest, SinglePointIntervals) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    vector_type single_points = {
        interval_type(1, 1, false, false),    // [1,1] - single point
        interval_type(3, 3, false, false),    // [3,3] - single point
        interval_type(5, 5, false, false),    // [5,5] - single point
        interval_type(3, 3, true, true)       // (3,3) - empty
    };

    auto result = make_disjoint_interval_set(single_points);

    // Should include the three non-empty single points
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0].left, 1);
    EXPECT_EQ(result[0].right, 1);
    EXPECT_EQ(result[1].left, 3);
    EXPECT_EQ(result[1].right, 3);
    EXPECT_EQ(result[2].left, 5);
    EXPECT_EQ(result[2].right, 5);
}

// Test type aliases
TEST(AlgorithmsTest, TypeAliases) {
    using interval_type = interval<int>;
    using vector_type = std::vector<interval_type>;

    // Test interval_type alias
    using extracted_interval = interval_type<vector_type>;
    static_assert(std::is_same_v<extracted_interval, interval_type>);

    // Test interval_value_type alias
    using extracted_value = interval_value_type<vector_type>;
    static_assert(std::is_same_v<extracted_value, int>);
}