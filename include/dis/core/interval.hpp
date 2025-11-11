#pragma once

#include <optional>
#include <concepts>
#include <compare>
#include <type_traits>
#include <limits>
#include <algorithm>

namespace dis {

// Concept for types that can be interval boundaries
template<typename T>
concept Boundary = std::totally_ordered<T> && std::regular<T>;

/**
 * @brief A mathematical interval over a totally ordered type T.
 *
 * This class models mathematical intervals with a clean, composable API.
 * It follows the principle of least surprise and provides multiple
 * equivalent ways to express operations for different use cases.
 *
 * Design principles:
 * - Immutable by default (functional style)
 * - Zero-cost abstractions (everything inline/constexpr)
 * - Clear semantics (empty intervals handled consistently)
 * - Composable operations (all operations return intervals)
 */
template<Boundary T>
class interval {
public:
    // Standard library-compatible type aliases
    using value_type = T;
    using difference_type = std::ptrdiff_t;

    // === Construction ===

    // Default constructor creates empty interval
    constexpr interval() noexcept
        : lower_(0), upper_(-1), left_closed_(false), right_closed_(false) {}

    // Generic constructor
    constexpr interval(T lower, T upper, bool left_closed = true, bool right_closed = true) noexcept
        : lower_(lower), upper_(upper), left_closed_(left_closed), right_closed_(right_closed) {
        // Normalize empty intervals
        if (lower_ > upper_ || (lower_ == upper_ && (!left_closed_ || !right_closed_))) {
            lower_ = 0;
            upper_ = -1;
            left_closed_ = false;
            right_closed_ = false;
        }
    }

    // === Named Constructors (Factory Methods) ===

    [[nodiscard]] static constexpr interval closed(T lower, T upper) noexcept {
        return interval(lower, upper, true, true);
    }

    [[nodiscard]] static constexpr interval open(T lower, T upper) noexcept {
        return interval(lower, upper, false, false);
    }

    [[nodiscard]] static constexpr interval left_open(T lower, T upper) noexcept {
        return interval(lower, upper, false, true);
    }

    [[nodiscard]] static constexpr interval right_open(T lower, T upper) noexcept {
        return interval(lower, upper, true, false);
    }

    [[nodiscard]] static constexpr interval point(T value) noexcept {
        return interval(value, value, true, true);
    }

    // Note: No static empty() factory - use default constructor interval()
    // This avoids name collision with the STL-standard empty() query method

    [[nodiscard]] static constexpr interval unbounded() noexcept
        requires std::numeric_limits<T>::has_infinity {
        return interval(-std::numeric_limits<T>::infinity(),
                       std::numeric_limits<T>::infinity(), false, false);
    }

    [[nodiscard]] static constexpr interval at_least(T lower) noexcept
        requires std::numeric_limits<T>::has_infinity {
        return interval(lower, std::numeric_limits<T>::infinity(), true, false);
    }

    [[nodiscard]] static constexpr interval at_most(T upper) noexcept
        requires std::numeric_limits<T>::has_infinity {
        return interval(-std::numeric_limits<T>::infinity(), upper, false, true);
    }

    [[nodiscard]] static constexpr interval greater_than(T lower) noexcept
        requires std::numeric_limits<T>::has_infinity {
        return interval(lower, std::numeric_limits<T>::infinity(), false, false);
    }

    [[nodiscard]] static constexpr interval less_than(T upper) noexcept
        requires std::numeric_limits<T>::has_infinity {
        return interval(-std::numeric_limits<T>::infinity(), upper, false, false);
    }

    // === Core Queries ===

    [[nodiscard]] constexpr bool empty() const noexcept {
        return lower_ > upper_;
    }

    [[nodiscard]] constexpr bool contains(T value) const noexcept {
        if (empty()) return false;
        const bool left_ok = left_closed_ ? (value >= lower_) : (value > lower_);
        const bool right_ok = right_closed_ ? (value <= upper_) : (value < upper_);
        return left_ok && right_ok;
    }

    [[nodiscard]] constexpr bool is_point() const noexcept {
        return !empty() && lower_ == upper_ && left_closed_ && right_closed_;
    }

    [[nodiscard]] constexpr bool is_bounded() const noexcept {
        if constexpr (std::numeric_limits<T>::has_infinity) {
            return !empty() &&
                   lower_ != -std::numeric_limits<T>::infinity() &&
                   upper_ != std::numeric_limits<T>::infinity();
        } else {
            return !empty();
        }
    }

    // === Boundary Access ===

    [[nodiscard]] constexpr std::optional<T> lower_bound() const noexcept {
        return empty() ? std::nullopt : std::optional<T>(lower_);
    }

    [[nodiscard]] constexpr std::optional<T> upper_bound() const noexcept {
        return empty() ? std::nullopt : std::optional<T>(upper_);
    }

    [[nodiscard]] constexpr bool is_left_closed() const noexcept {
        return !empty() && left_closed_;
    }

    [[nodiscard]] constexpr bool is_right_closed() const noexcept {
        return !empty() && right_closed_;
    }

    // === Set Relations ===

    [[nodiscard]] constexpr bool subset_of(const interval& other) const noexcept {
        if (empty()) return true;
        if (other.empty()) return false;

        const bool left_ok = (other.lower_ < lower_) ||
            (other.lower_ == lower_ && (!other.left_closed_ || left_closed_));
        const bool right_ok = (other.upper_ > upper_) ||
            (other.upper_ == upper_ && (!other.right_closed_ || right_closed_));

        return left_ok && right_ok;
    }

    [[nodiscard]] constexpr bool superset_of(const interval& other) const noexcept {
        return other.subset_of(*this);
    }

    [[nodiscard]] constexpr bool overlaps(const interval& other) const noexcept {
        if (empty() || other.empty()) return false;

        if (upper_ < other.lower_) return false;
        if (lower_ > other.upper_) return false;
        if (upper_ == other.lower_) return right_closed_ && other.left_closed_;
        if (lower_ == other.upper_) return left_closed_ && other.right_closed_;

        return true;
    }

    [[nodiscard]] constexpr bool disjoint_from(const interval& other) const noexcept {
        return !overlaps(other);
    }

    [[nodiscard]] constexpr bool adjacent_to(const interval& other) const noexcept {
        if (empty() || other.empty()) return false;

        if (upper_ == other.lower_) return right_closed_ != other.left_closed_;
        if (lower_ == other.upper_) return left_closed_ != other.right_closed_;

        return false;
    }

    // === Set Operations ===

    [[nodiscard]] constexpr interval intersect(const interval& other) const noexcept {
        if (empty() || other.empty()) return interval{};

        const T new_lower = std::max(lower_, other.lower_);
        const T new_upper = std::min(upper_, other.upper_);

        if (new_lower > new_upper) return interval{};

        const bool new_left = (lower_ == other.lower_)
            ? (left_closed_ && other.left_closed_)
            : (new_lower == lower_ ? left_closed_ : other.left_closed_);

        const bool new_right = (upper_ == other.upper_)
            ? (right_closed_ && other.right_closed_)
            : (new_upper == upper_ ? right_closed_ : other.right_closed_);

        if (new_lower == new_upper && (!new_left || !new_right)) return interval{};

        return interval(new_lower, new_upper, new_left, new_right);
    }

    [[nodiscard]] constexpr std::optional<interval> hull(const interval& other) const noexcept {
        if (empty()) return other;
        if (other.empty()) return *this;

        // Can only create hull if intervals overlap or are adjacent
        if (!overlaps(other) && !adjacent_to(other)) {
            return std::nullopt;
        }

        const T new_lower = std::min(lower_, other.lower_);
        const T new_upper = std::max(upper_, other.upper_);

        const bool new_left = (lower_ == other.lower_)
            ? (left_closed_ || other.left_closed_)
            : (new_lower == lower_ ? left_closed_ : other.left_closed_);

        const bool new_right = (upper_ == other.upper_)
            ? (right_closed_ || other.right_closed_)
            : (new_upper == upper_ ? right_closed_ : other.right_closed_);

        return interval(new_lower, new_upper, new_left, new_right);
    }

    // === Measure Operations ===

    [[nodiscard]] constexpr T length() const noexcept
        requires std::is_arithmetic_v<T> {
        return empty() ? T(0) : (upper_ - lower_);
    }

    [[nodiscard]] constexpr T midpoint() const noexcept
        requires std::is_arithmetic_v<T> {
        return empty() ? T(0) : lower_ + length() / T(2);
    }

    [[nodiscard]] constexpr T distance_to(const interval& other) const noexcept
        requires std::is_arithmetic_v<T> {
        if (empty() || other.empty()) return T(0);
        if (overlaps(other)) return T(0);

        if (upper_ < other.lower_) {
            return other.lower_ - upper_;
        } else {
            return lower_ - other.upper_;
        }
    }

    // === Comparison Operators ===

    [[nodiscard]] friend constexpr bool operator==(const interval& a, const interval& b) noexcept {
        if (a.empty() && b.empty()) return true;
        return a.lower_ == b.lower_ && a.upper_ == b.upper_ &&
               a.left_closed_ == b.left_closed_ && a.right_closed_ == b.right_closed_;
    }

    [[nodiscard]] friend constexpr auto operator<=>(const interval& a, const interval& b) noexcept {
        // Lexicographic ordering for use in containers
        // Use the ordering category of the underlying type T
        using ordering = std::compare_three_way_result_t<T>;

        if (a.empty() && b.empty()) return ordering::equivalent;
        if (a.empty()) return ordering::less;
        if (b.empty()) return ordering::greater;

        if (auto cmp = a.lower_ <=> b.lower_; cmp != 0) return cmp;
        if (a.left_closed_ != b.left_closed_) {
            return a.left_closed_ ? ordering::less : ordering::greater;
        }
        if (auto cmp = a.upper_ <=> b.upper_; cmp != 0) return cmp;
        if (a.right_closed_ != b.right_closed_) {
            return a.right_closed_ ? ordering::less : ordering::greater;
        }
        return ordering::equivalent;
    }

    // === Mathematical Operator Overloads ===

    [[nodiscard]] friend constexpr interval operator&(const interval& a, const interval& b) noexcept {
        return a.intersect(b);
    }

    [[nodiscard]] friend constexpr interval operator*(const interval& a, const interval& b) noexcept {
        return a.intersect(b);
    }

private:
    T lower_, upper_;
    bool left_closed_, right_closed_;
};

// Type aliases for common use cases
using real_interval = interval<double>;
using integer_interval = interval<int>;

} // namespace dis