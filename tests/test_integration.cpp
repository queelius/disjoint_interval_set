#include <gtest/gtest.h>
#include <disjoint_interval_set/disjoint_interval_set.hpp>
#include <disjoint_interval_set/disjoint_interval_set_algorithms.hpp>
#include <disjoint_interval_set/interval.hpp>
#include <vector>
#include <algorithm>
#include <random>

using namespace disjoint_interval_set;

// Integration test for complex real-world scenarios
TEST(IntegrationTest, TimeSchedulingScenario) {
    // Simulate a scheduling system where intervals represent busy times
    using time_interval = interval<double>;  // Hours as doubles
    using schedule = disjoint_interval_set<time_interval>;

    // Person A's busy times
    schedule personA;
    personA = personA + schedule{{time_interval(9.0, 10.5, false, false)}};   // Meeting 9:00-10:30
    personA = personA + schedule{{time_interval(11.0, 12.0, false, false)}};  // Lunch 11:00-12:00
    personA = personA + schedule{{time_interval(14.0, 15.5, false, false)}};  // Meeting 14:00-15:30

    // Person B's busy times
    schedule personB;
    personB = personB + schedule{{time_interval(8.5, 9.5, false, false)}};    // Meeting 8:30-9:30
    personB = personB + schedule{{time_interval(11.5, 13.0, false, false)}};  // Lunch 11:30-13:00
    personB = personB + schedule{{time_interval(15.0, 16.0, false, false)}};  // Meeting 15:00-16:00

    // Find when both are busy (intersection)
    auto both_busy = personA * personB;

    // Check specific overlap times
    EXPECT_TRUE(both_busy.contains(9.25));   // Both busy 9:00-9:30
    EXPECT_FALSE(both_busy.contains(10.0));  // Only A busy
    EXPECT_FALSE(both_busy.contains(13.5));  // Neither busy
    EXPECT_TRUE(both_busy.contains(15.25));  // Both busy 15:00-15:30

    // Find when either is busy (union)
    auto either_busy = personA + personB;

    EXPECT_TRUE(either_busy.contains(8.75));  // B busy
    EXPECT_TRUE(either_busy.contains(9.25));  // Both busy
    EXPECT_TRUE(either_busy.contains(10.0));  // A busy
    EXPECT_TRUE(either_busy.contains(12.5));  // B busy
    EXPECT_FALSE(either_busy.contains(13.5)); // Neither busy
    EXPECT_TRUE(either_busy.contains(15.25)); // Both busy

    // Find free time slots (complement within working hours)
    // Assuming work day is 8:00-17:00
    schedule work_day;
    work_day = work_day + schedule{{time_interval(8.0, 17.0, false, false)}};

    auto personA_free = work_day - personA;
    auto personB_free = work_day - personB;

    // Find common free time (when both are free)
    auto common_free = personA_free * personB_free;

    EXPECT_TRUE(common_free.contains(8.25));  // Both free early morning
    EXPECT_FALSE(common_free.contains(9.25)); // Both busy
    EXPECT_TRUE(common_free.contains(13.5));  // Both free after lunch
    EXPECT_FALSE(common_free.contains(15.25)); // Both busy
    EXPECT_TRUE(common_free.contains(16.5));  // Both free late afternoon
}

TEST(IntegrationTest, NumberRangeFiltering) {
    // Test filtering number ranges with various conditions
    using range = disjoint_interval_set<interval<int>>;

    // Define valid ranges
    range valid_ages;
    valid_ages = valid_ages + range{{interval<int>(18, 65, false, false)}};  // Working age

    range valid_scores;
    valid_scores = valid_scores + range{{interval<int>(0, 100, false, false)}};  // Test scores

    // Define some special ranges
    range prime_decades;  // Ages in 20s, 30s, 50s
    prime_decades = prime_decades + range{{interval<int>(20, 29, false, false)}};
    prime_decades = prime_decades + range{{interval<int>(30, 39, false, false)}};
    prime_decades = prime_decades + range{{interval<int>(50, 59, false, false)}};

    // Find working age people in prime decades
    auto target_demographic = valid_ages * prime_decades;

    EXPECT_TRUE(target_demographic.contains(25));   // 20s
    EXPECT_TRUE(target_demographic.contains(35));   // 30s
    EXPECT_FALSE(target_demographic.contains(45));  // 40s not included
    EXPECT_TRUE(target_demographic.contains(55));   // 50s
    EXPECT_FALSE(target_demographic.contains(65));  // Too old

    // Define invalid ranges (complement of valid)
    auto invalid_ages = ~valid_ages;
    EXPECT_TRUE(invalid_ages.contains(17));   // Too young
    EXPECT_FALSE(invalid_ages.contains(30));  // Valid age
    EXPECT_TRUE(invalid_ages.contains(70));   // Too old

    // Complex filtering: valid scores excluding certain ranges
    range excluded_scores;
    excluded_scores = excluded_scores + range{{interval<int>(40, 49, false, false)}};  // D grades

    auto good_scores = valid_scores - excluded_scores;
    EXPECT_TRUE(good_scores.contains(39));   // Just below excluded
    EXPECT_FALSE(good_scores.contains(45));  // In excluded range
    EXPECT_TRUE(good_scores.contains(50));   // Just above excluded
}

TEST(IntegrationTest, GeometricRegions) {
    // Test with geometric regions (1D projections)
    using coord = double;
    using region = disjoint_interval_set<interval<coord>>;

    // Define regions on X-axis
    region obstacle1;
    obstacle1 = obstacle1 + region{{interval<coord>(-5.0, -2.0, false, false)}};
    obstacle1 = obstacle1 + region{{interval<coord>(2.0, 5.0, false, false)}};

    region obstacle2;
    obstacle2 = obstacle2 + region{{interval<coord>(-3.0, -1.0, false, false)}};
    obstacle2 = obstacle2 + region{{interval<coord>(1.0, 3.0, false, false)}};

    // Find union of obstacles (all blocked regions)
    auto all_obstacles = obstacle1 + obstacle2;

    // Check merged regions
    EXPECT_TRUE(all_obstacles.contains(-4.0));   // In obstacle1
    EXPECT_TRUE(all_obstacles.contains(-2.5));   // In both
    EXPECT_TRUE(all_obstacles.contains(2.5));    // In both
    EXPECT_FALSE(all_obstacles.contains(0.0));   // Free space

    // Find overlapping regions (collision zones)
    auto collision_zones = obstacle1 * obstacle2;

    EXPECT_FALSE(collision_zones.contains(-4.0)); // Only obstacle1
    EXPECT_TRUE(collision_zones.contains(-2.5));  // Both obstacles
    EXPECT_TRUE(collision_zones.contains(2.5));   // Both obstacles
    EXPECT_FALSE(collision_zones.contains(4.0));  // Only obstacle1

    // Define safe path region
    region safe_path;
    safe_path = safe_path + region{{interval<coord>(-10.0, 10.0, false, false)}};

    // Find free space (safe path minus obstacles)
    auto free_space = safe_path - all_obstacles;

    EXPECT_FALSE(free_space.contains(-2.5));  // Blocked
    EXPECT_TRUE(free_space.contains(0.0));    // Free
    EXPECT_FALSE(free_space.contains(2.5));   // Blocked
    EXPECT_TRUE(free_space.contains(7.0));    // Free
}

TEST(IntegrationTest, SetAlgebraProperties) {
    // Verify fundamental set algebra properties
    using iset = disjoint_interval_set<interval<int>>;

    // Create test sets
    iset A, B, C;
    A = A + iset{{interval<int>(1, 5, false, false)}};
    A = A + iset{{interval<int>(10, 15, false, false)}};

    B = B + iset{{interval<int>(3, 8, false, false)}};
    B = B + iset{{interval<int>(12, 18, false, false)}};

    C = C + iset{{interval<int>(0, 20, false, false)}};

    // Test commutative property of union: A ∪ B = B ∪ A
    auto AunionB = A + B;
    auto BunionA = B + A;
    EXPECT_EQ(AunionB, BunionA);

    // Test commutative property of intersection: A ∩ B = B ∩ A
    auto AintersectB = A * B;
    auto BintersectA = B * A;
    EXPECT_EQ(AintersectB, BintersectA);

    // Test associative property of union: (A ∪ B) ∪ C = A ∪ (B ∪ C)
    auto AB_union_C = (A + B) + C;
    auto A_union_BC = A + (B + C);
    EXPECT_EQ(AB_union_C, A_union_BC);

    // Test associative property of intersection: (A ∩ B) ∩ C = A ∩ (B ∩ C)
    auto AB_intersect_C = (A * B) * C;
    auto A_intersect_BC = A * (B * C);
    EXPECT_EQ(AB_intersect_C, A_intersect_BC);

    // Test distributive property: A ∩ (B ∪ C) = (A ∩ B) ∪ (A ∩ C)
    auto BunionC = B + C;
    auto A_intersect_BunionC = A * BunionC;
    auto AintersectB_union_AintersectC = (A * B) + (A * C);
    EXPECT_EQ(A_intersect_BunionC, AintersectB_union_AintersectC);

    // Test De Morgan's Law: ~(A ∪ B) = ~A ∩ ~B
    auto not_AunionB = ~(A + B);
    auto notA_intersect_notB = (~A) * (~B);
    // Check equality at sample points
    for (int i = -5; i <= 25; ++i) {
        EXPECT_EQ(not_AunionB.contains(i), notA_intersect_notB.contains(i))
            << "De Morgan's Law failed at point " << i;
    }

    // Test identity properties
    iset empty_set;
    auto A_union_empty = A + empty_set;
    EXPECT_EQ(A_union_empty, A);  // A ∪ ∅ = A

    auto A_intersect_C = A * C;
    EXPECT_EQ(A_intersect_C, A);  // A ∩ U = A (where C contains A)

    // Test idempotent properties
    auto A_union_A = A + A;
    EXPECT_EQ(A_union_A, A);  // A ∪ A = A

    auto A_intersect_A = A * A;
    EXPECT_EQ(A_intersect_A, A);  // A ∩ A = A

    // Test complement properties
    auto A_union_notA = A + (~A);
    auto A_intersect_notA = A * (~A);
    EXPECT_TRUE(A_intersect_notA.empty());  // A ∩ ~A = ∅
}

TEST(IntegrationTest, BoundaryConditions) {
    // Test various boundary condition combinations
    using dset = disjoint_interval_set<interval<double>>;

    dset open_set;
    open_set = open_set + dset{{interval<double>(1.0, 2.0, true, true)}};    // (1,2)
    open_set = open_set + dset{{interval<double>(3.0, 4.0, true, true)}};    // (3,4)

    dset closed_set;
    closed_set = closed_set + dset{{interval<double>(1.5, 2.5, false, false)}};  // [1.5,2.5]
    closed_set = closed_set + dset{{interval<double>(3.5, 4.5, false, false)}};  // [3.5,4.5]

    dset mixed_set;
    mixed_set = mixed_set + dset{{interval<double>(1.0, 2.0, false, true)}};   // [1,2)
    mixed_set = mixed_set + dset{{interval<double>(2.0, 3.0, false, true)}};   // [2,3)

    // Test intersections with different boundaries
    auto open_closed_intersect = open_set * closed_set;
    EXPECT_TRUE(open_closed_intersect.contains(1.75));  // In (1,2) ∩ [1.5,2.5]
    EXPECT_FALSE(open_closed_intersect.contains(1.0));  // Boundary excluded by open
    EXPECT_FALSE(open_closed_intersect.contains(2.0));  // Boundary excluded by open

    // Test union of adjacent intervals with compatible boundaries
    EXPECT_TRUE(mixed_set.contains(1.5));   // In [1,2)
    EXPECT_TRUE(mixed_set.contains(2.5));   // In [2,3)
    EXPECT_FALSE(mixed_set.contains(3.0));  // Right boundary open

    // Count intervals after operations
    int mixed_count = 0;
    for (auto it = mixed_set.begin(); it != mixed_set.end(); ++it) {
        mixed_count++;
    }
    EXPECT_EQ(mixed_count, 1);  // Should merge to single interval [1,3)
}

TEST(IntegrationTest, PerformanceStressTest) {
    // Test with large number of intervals
    using iset = disjoint_interval_set<interval<int>>;

    // Create set with many intervals
    iset large_set;
    for (int i = 0; i < 100; i += 3) {
        large_set = large_set + iset{{interval<int>(i, i + 1, false, false)}};
    }

    // Verify set properties
    EXPECT_TRUE(large_set.contains(0));
    EXPECT_TRUE(large_set.contains(3));
    EXPECT_FALSE(large_set.contains(2));
    EXPECT_TRUE(large_set.contains(99));
    EXPECT_FALSE(large_set.contains(100));

    // Create another large set
    iset large_set2;
    for (int i = 1; i < 100; i += 3) {
        large_set2 = large_set2 + iset{{interval<int>(i, i + 1, false, false)}};
    }

    // Test operations on large sets
    auto large_union = large_set + large_set2;
    auto large_intersection = large_set * large_set2;
    auto large_difference = large_set - large_set2;

    // Verify operations
    EXPECT_TRUE(large_union.contains(0));
    EXPECT_TRUE(large_union.contains(1));
    EXPECT_TRUE(large_union.contains(3));

    EXPECT_TRUE(large_intersection.empty());  // No overlaps

    EXPECT_EQ(large_difference, large_set);  // No overlaps, so difference equals original
}

TEST(IntegrationTest, EmptySetOperations) {
    // Comprehensive test of empty set behaviors
    using iset = disjoint_interval_set<interval<int>>;

    iset empty1, empty2;
    iset non_empty;
    non_empty = non_empty + iset{{interval<int>(1, 5, false, false)}};

    // Empty set operations
    EXPECT_TRUE(empty1.empty());
    EXPECT_TRUE(empty2.empty());
    EXPECT_FALSE(empty1.infimum().has_value());
    EXPECT_FALSE(empty1.supremum().has_value());

    // Operations with empty sets
    auto empty_union = empty1 + empty2;
    EXPECT_TRUE(empty_union.empty());

    auto empty_intersect = empty1 * empty2;
    EXPECT_TRUE(empty_intersect.empty());

    auto empty_diff = empty1 - empty2;
    EXPECT_TRUE(empty_diff.empty());

    auto empty_sym_diff = empty1 ^ empty2;
    EXPECT_TRUE(empty_sym_diff.empty());

    // Operations between empty and non-empty
    auto union_with_empty = non_empty + empty1;
    EXPECT_EQ(union_with_empty, non_empty);

    auto intersect_with_empty = non_empty * empty1;
    EXPECT_TRUE(intersect_with_empty.empty());

    auto diff_with_empty = non_empty - empty1;
    EXPECT_EQ(diff_with_empty, non_empty);

    // Complement of empty set
    auto complement_empty = ~empty1;
    EXPECT_TRUE(complement_empty.contains(0));
    EXPECT_TRUE(complement_empty.contains(-1000));
    EXPECT_TRUE(complement_empty.contains(1000));

    // Relations with empty set
    EXPECT_TRUE(empty1 <= empty2);   // Empty ⊆ Empty
    EXPECT_TRUE(empty1 >= empty2);   // Empty ⊇ Empty
    EXPECT_TRUE(empty1 == empty2);   // Empty = Empty
    EXPECT_FALSE(empty1 != empty2);  // !(Empty ≠ Empty)
    EXPECT_FALSE(empty1 < empty2);   // !(Empty ⊂ Empty)
    EXPECT_FALSE(empty1 > empty2);   // !(Empty ⊃ Empty)

    EXPECT_TRUE(empty1 <= non_empty);   // Empty ⊆ Non-empty
    EXPECT_FALSE(non_empty <= empty1);  // !(Non-empty ⊆ Empty)
}

TEST(IntegrationTest, RealWorldIPRanges) {
    // Simulate IP address range management (using integers for simplicity)
    using ip_range = disjoint_interval_set<interval<int>>;

    // Private IP ranges (simplified as integer ranges)
    ip_range private_class_a;  // 10.0.0.0 - 10.255.255.255
    private_class_a = private_class_a + ip_range{{interval<int>(10000, 10255, false, false)}};

    ip_range private_class_b;  // 172.16.0.0 - 172.31.255.255
    private_class_b = private_class_b + ip_range{{interval<int>(17216, 17231, false, false)}};

    ip_range private_class_c;  // 192.168.0.0 - 192.168.255.255
    private_class_c = private_class_c + ip_range{{interval<int>(19216, 19216, false, false)}};

    // All private ranges
    auto all_private = private_class_a + private_class_b + private_class_c;

    // Company allocated ranges
    ip_range company_allocated;
    company_allocated = company_allocated + ip_range{{interval<int>(10100, 10150, false, false)}};
    company_allocated = company_allocated + ip_range{{interval<int>(17220, 17225, false, false)}};

    // Find available private IPs (private ranges minus allocated)
    auto available_private = all_private - company_allocated;

    // Verify allocations
    EXPECT_FALSE(available_private.contains(10125));  // Allocated
    EXPECT_TRUE(available_private.contains(10200));   // Available
    EXPECT_FALSE(available_private.contains(17222));  // Allocated
    EXPECT_TRUE(available_private.contains(17230));   // Available

    // Check if a range is fully within private space
    ip_range test_range;
    test_range = test_range + ip_range{{interval<int>(10120, 10130, false, false)}};

    bool is_private = (test_range <= all_private);
    EXPECT_TRUE(is_private);

    // Check for conflicts with allocated ranges
    auto conflicts = test_range * company_allocated;
    EXPECT_FALSE(conflicts.empty());  // Has conflicts
}

// Test with custom interval types
class CustomInterval : public interval<double> {
public:
    CustomInterval() : interval<double>() {}
    CustomInterval(double l, double r, bool lo = false, bool ro = false)
        : interval<double>(l, r, lo, ro) {}
};

TEST(IntegrationTest, CustomIntervalType) {
    using custom_set = disjoint_interval_set<CustomInterval>;

    custom_set set1;
    set1 = set1 + custom_set{{CustomInterval(1.0, 3.0)}};
    set1 = set1 + custom_set{{CustomInterval(5.0, 7.0)}};

    custom_set set2;
    set2 = set2 + custom_set{{CustomInterval(2.0, 4.0)}};
    set2 = set2 + custom_set{{CustomInterval(6.0, 8.0)}};

    auto union_result = set1 + set2;
    auto intersect_result = set1 * set2;

    // Verify operations work with custom type
    EXPECT_TRUE(union_result.contains(1.5));
    EXPECT_TRUE(union_result.contains(3.5));
    EXPECT_TRUE(union_result.contains(7.5));

    EXPECT_TRUE(intersect_result.contains(2.5));
    EXPECT_FALSE(intersect_result.contains(1.5));
    EXPECT_TRUE(intersect_result.contains(6.5));
}