#pragma once

#include <type_traits>
#include <limits>
#include <optional>

namespace disjoint_interval_set {

/**
 * @brief Traits class for interval boundary types.
 *
 * Provides customization points for different numeric types,
 * allowing users to extend the library for custom types.
 */
template<typename T>
struct boundary_traits {
    using value_type = T;

    // Default infinity values for numeric types
    static constexpr T min() noexcept {
        if constexpr (std::numeric_limits<T>::has_infinity) {
            return -std::numeric_limits<T>::infinity();
        } else if constexpr (std::numeric_limits<T>::is_integer) {
            return std::numeric_limits<T>::min();
        } else {
            return std::numeric_limits<T>::lowest();
        }
    }

    static constexpr T max() noexcept {
        if constexpr (std::numeric_limits<T>::has_infinity) {
            return std::numeric_limits<T>::infinity();
        } else {
            return std::numeric_limits<T>::max();
        }
    }

    // Check if a value represents negative infinity
    static constexpr bool is_min(T value) noexcept {
        return value == min();
    }

    // Check if a value represents positive infinity
    static constexpr bool is_max(T value) noexcept {
        return value == max();
    }

    // Get the next value in the domain (for discrete types)
    static constexpr std::optional<T> next(T value) noexcept {
        if constexpr (std::numeric_limits<T>::is_integer) {
            if (value == max()) return std::nullopt;
            return value + 1;
        } else {
            return std::nullopt;  // No next value for continuous types
        }
    }

    // Get the previous value in the domain (for discrete types)
    static constexpr std::optional<T> prev(T value) noexcept {
        if constexpr (std::numeric_limits<T>::is_integer) {
            if (value == min()) return std::nullopt;
            return value - 1;
        } else {
            return std::nullopt;  // No previous value for continuous types
        }
    }

    // Check if the domain is discrete
    static constexpr bool is_discrete() noexcept {
        return std::numeric_limits<T>::is_integer;
    }

    // Check if two boundaries are adjacent in a discrete domain
    static constexpr bool are_adjacent(T a, T b) noexcept {
        if constexpr (std::numeric_limits<T>::is_integer) {
            return (b == a + 1) || (a == b + 1);
        } else {
            return false;  // No adjacency in continuous domains
        }
    }
};

/**
 * @brief Traits class for interval types.
 *
 * Provides compile-time information about interval types,
 * enabling generic algorithms and optimizations.
 */
template<typename I>
struct interval_traits {
    using interval_type = I;
    using value_type = typename I::value_type;
    using boundary_type = boundary_traits<value_type>;

    // Check if the interval type supports open/closed boundaries
    static constexpr bool has_boundary_types = requires(const I& i) {
        { i.left_open } -> std::convertible_to<bool>;
        { i.right_open } -> std::convertible_to<bool>;
    };

    // Check if the interval type is compile-time static
    static constexpr bool is_static = false;

    // Get the universal interval (contains all values)
    static I universal() {
        if constexpr (has_boundary_types) {
            return I(boundary_type::min(), boundary_type::max(), false, false);
        } else {
            return I(boundary_type::min(), boundary_type::max());
        }
    }

    // Get the empty interval
    static I empty() {
        return I{};  // Assumes default constructor creates empty interval
    }

    // Create a singleton interval containing a single point
    static I singleton(value_type value) {
        if constexpr (has_boundary_types) {
            return I(value, value, false, false);
        } else {
            return I(value, value);
        }
    }

    // Create an open interval
    static I open(value_type lower, value_type upper) {
        if constexpr (has_boundary_types) {
            return I(lower, upper, true, true);
        } else {
            static_assert(has_boundary_types, "Interval type doesn't support open boundaries");
        }
    }

    // Create a closed interval
    static I closed(value_type lower, value_type upper) {
        if constexpr (has_boundary_types) {
            return I(lower, upper, false, false);
        } else {
            return I(lower, upper);
        }
    }

    // Create a left-open interval [lower, upper)
    static I left_open(value_type lower, value_type upper) {
        if constexpr (has_boundary_types) {
            return I(lower, upper, true, false);
        } else {
            static_assert(has_boundary_types, "Interval type doesn't support mixed boundaries");
        }
    }

    // Create a right-open interval (lower, upper]
    static I right_open(value_type lower, value_type upper) {
        if constexpr (has_boundary_types) {
            return I(lower, upper, false, true);
        } else {
            static_assert(has_boundary_types, "Interval type doesn't support mixed boundaries");
        }
    }
};

// Forward declaration for static_interval
template<typename T, T Lower, T Upper, bool LeftOpen, bool RightOpen>
struct static_interval;

/**
 * @brief Specialization for static interval types.
 *
 * Provides compile-time traits for static intervals.
 */
template<typename T, T Lower, T Upper, bool LeftOpen, bool RightOpen>
struct interval_traits<static_interval<T, Lower, Upper, LeftOpen, RightOpen>> {
    using interval_type = static_interval<T, Lower, Upper, LeftOpen, RightOpen>;
    using value_type = T;
    using boundary_type = boundary_traits<T>;

    static constexpr bool has_boundary_types = true;
    static constexpr bool is_static = true;

    static constexpr T lower = Lower;
    static constexpr T upper = Upper;
    static constexpr bool left_open = LeftOpen;
    static constexpr bool right_open = RightOpen;

    static constexpr bool is_empty = Lower > Upper || (Lower == Upper && (LeftOpen || RightOpen));
};

} // namespace disjoint_interval_set