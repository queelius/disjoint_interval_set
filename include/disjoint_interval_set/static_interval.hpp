#pragma once

#include <utility>
#include <array>
#include <algorithm>

namespace disjoint_interval_set {

/**
 * @brief Static (compile-time) interval with fixed bounds
 *
 * This class enables compile-time interval arithmetic operations,
 * allowing for zero-overhead abstractions and compile-time range validation.
 */
template<typename T, T Lower, T Upper, bool LeftOpen = false, bool RightOpen = false>
struct static_interval {
    static constexpr T lower = Lower;
    static constexpr T upper = Upper;
    static constexpr bool left_open = LeftOpen;
    static constexpr bool right_open = RightOpen;

    using value_type = T;

    // Check if interval is empty at compile-time
    static constexpr bool is_empty() {
        return lower > upper || (lower == upper && (left_open || right_open));
    }

    // Check if value is contained in interval at compile-time
    static constexpr bool contains(T value) {
        if (is_empty()) return false;
        bool left_ok = left_open ? (value > lower) : (value >= lower);
        bool right_ok = right_open ? (value < upper) : (value <= upper);
        return left_ok && right_ok;
    }

    // Intersection with another static interval
    template<T OtherLower, T OtherUpper, bool OtherLeftOpen, bool OtherRightOpen>
    using intersect = std::conditional_t<
        (Lower > OtherUpper || OtherLower > Upper),
        static_interval<T, 0, -1, true, true>,  // Empty interval
        static_interval<T,
            (Lower > OtherLower) ? Lower : OtherLower,
            (Upper < OtherUpper) ? Upper : OtherUpper,
            (Lower > OtherLower) ? LeftOpen :
                (OtherLower > Lower) ? OtherLeftOpen : (LeftOpen || OtherLeftOpen),
            (Upper < OtherUpper) ? RightOpen :
                (OtherUpper < Upper) ? OtherRightOpen : (RightOpen || OtherRightOpen)
        >
    >;

    // Union requires result to be a set, handled by static_interval_set
};

/**
 * @brief Compile-time disjoint interval set
 *
 * Represents a set of non-overlapping intervals that can be manipulated
 * at compile-time.
 */
template<typename T, typename... Intervals>
class static_interval_set;

// Base case: empty set
template<typename T>
class static_interval_set<T> {
public:
    using value_type = T;
    static constexpr size_t size = 0;

    static constexpr bool is_empty() { return true; }

    static constexpr bool contains(T) { return false; }
};

// Recursive case: set with intervals
template<typename T, T L1, T U1, bool LO1, bool RO1, typename... Rest>
class static_interval_set<T, static_interval<T, L1, U1, LO1, RO1>, Rest...> {
public:
    using value_type = T;
    using first_interval = static_interval<T, L1, U1, LO1, RO1>;
    using rest_set = static_interval_set<T, Rest...>;

    static constexpr size_t size = 1 + rest_set::size;

    static constexpr bool is_empty() {
        return first_interval::is_empty() && rest_set::is_empty();
    }

    static constexpr bool contains(T value) {
        return first_interval::contains(value) || rest_set::contains(value);
    }
};

// Helper to create static intervals
template<typename T, T Lower, T Upper, bool LeftOpen = false, bool RightOpen = false>
constexpr auto make_static_interval() {
    return static_interval<T, Lower, Upper, LeftOpen, RightOpen>{};
}

// Union operation for static interval sets
template<typename T, typename... Is1, typename... Is2>
constexpr auto operator+(static_interval_set<T, Is1...>, static_interval_set<T, Is2...>) {
    return static_interval_set<T, Is1..., Is2...>{};
}

// Compile-time min/max functions for C++14 compatibility
template<typename T>
constexpr T static_min(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
constexpr T static_max(T a, T b) {
    return (a > b) ? a : b;
}

// Static interval validation at compile-time
template<typename T, T Lower, T Upper>
struct validate_interval {
    static_assert(Lower <= Upper, "Invalid interval: lower bound must be <= upper bound");
    static constexpr bool valid = true;
};

} // namespace disjoint_interval_set