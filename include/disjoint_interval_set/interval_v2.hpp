#pragma once

#include <optional>
#include <compare>
#include <utility>
#include <concepts>
#include "concepts.hpp"
#include "interval_traits.hpp"

namespace disjoint_interval_set {

/**
 * @brief Modern interval implementation with improved API design.
 *
 * This class models mathematical intervals with elegant API that follows
 * STL conventions while maintaining mathematical rigor.
 *
 * Key improvements:
 * - Consistent API with both member and free functions
 * - Named factory methods for better readability
 * - Move semantics and noexcept specifications
 * - Three-way comparison support (C++20)
 * - Builder pattern for complex interval construction
 */
template<BoundaryType T>
class interval {
public:
    using value_type = T;
    using traits = interval_traits<interval>;
    using boundary_traits_type = boundary_traits<T>;

    // Constructors
    constexpr interval() noexcept
        : lower_(0), upper_(-1), left_open_(true), right_open_(true) {}

    constexpr interval(T lower, T upper,
                      bool left_open = false, bool right_open = false) noexcept
        : lower_(lower), upper_(upper),
          left_open_(left_open), right_open_(right_open) {}

    constexpr interval(const interval&) noexcept = default;
    constexpr interval(interval&&) noexcept = default;

    constexpr interval& operator=(const interval&) noexcept = default;
    constexpr interval& operator=(interval&&) noexcept = default;

    // Named factory methods for clarity
    [[nodiscard]] static constexpr interval closed(T lower, T upper) noexcept {
        return interval(lower, upper, false, false);
    }

    [[nodiscard]] static constexpr interval open(T lower, T upper) noexcept {
        return interval(lower, upper, true, true);
    }

    [[nodiscard]] static constexpr interval left_open(T lower, T upper) noexcept {
        return interval(lower, upper, true, false);
    }

    [[nodiscard]] static constexpr interval right_open(T lower, T upper) noexcept {
        return interval(lower, upper, false, true);
    }

    [[nodiscard]] static constexpr interval singleton(T value) noexcept {
        return interval(value, value, false, false);
    }

    [[nodiscard]] static constexpr interval make_empty() noexcept {
        return interval();
    }

    [[nodiscard]] static constexpr interval universal() noexcept {
        return interval(boundary_traits_type::min(),
                       boundary_traits_type::max(), false, false);
    }

    // Core predicates
    [[nodiscard]] constexpr bool is_empty() const noexcept {
        return lower_ > upper_ ||
               (lower_ == upper_ && (left_open_ || right_open_));
    }

    [[nodiscard]] constexpr bool contains(T value) const noexcept {
        if (is_empty()) return false;
        const bool left_ok = left_open_ ? (value > lower_) : (value >= lower_);
        const bool right_ok = right_open_ ? (value < upper_) : (value <= upper_);
        return left_ok && right_ok;
    }

    [[nodiscard]] constexpr bool is_singleton() const noexcept {
        return !is_empty() && lower_ == upper_ && !left_open_ && !right_open_;
    }

    [[nodiscard]] constexpr bool is_bounded() const noexcept {
        return !boundary_traits_type::is_min(lower_) &&
               !boundary_traits_type::is_max(upper_);
    }

    // Boundary accessors
    [[nodiscard]] constexpr std::optional<T> lower() const noexcept {
        return is_empty() ? std::nullopt : std::optional<T>(lower_);
    }

    [[nodiscard]] constexpr std::optional<T> upper() const noexcept {
        return is_empty() ? std::nullopt : std::optional<T>(upper_);
    }

    [[nodiscard]] constexpr bool is_left_open() const noexcept {
        return left_open_;
    }

    [[nodiscard]] constexpr bool is_right_open() const noexcept {
        return right_open_;
    }

    [[nodiscard]] constexpr bool is_left_closed() const noexcept {
        return !left_open_;
    }

    [[nodiscard]] constexpr bool is_right_closed() const noexcept {
        return !right_open_;
    }

    // Mathematical operations as member functions
    [[nodiscard]] constexpr interval intersect(const interval& other) const noexcept {
        if (is_empty() || other.is_empty()) return make_empty();

        const T new_lower = std::max(lower_, other.lower_);
        const T new_upper = std::min(upper_, other.upper_);

        if (new_lower > new_upper) return make_empty();

        const bool new_left_open = (lower_ == other.lower_)
            ? (left_open_ || other.left_open_)
            : (new_lower == lower_ ? left_open_ : other.left_open_);

        const bool new_right_open = (upper_ == other.upper_)
            ? (right_open_ || other.right_open_)
            : (new_upper == upper_ ? right_open_ : other.right_open_);

        return interval(new_lower, new_upper, new_left_open, new_right_open);
    }

    [[nodiscard]] constexpr std::optional<interval> unite(const interval& other) const noexcept {
        if (is_empty()) return other;
        if (other.is_empty()) return *this;

        // Can only unite if intervals overlap or are adjacent
        if (!overlaps(other) && !adjacent(other)) {
            return std::nullopt;
        }

        const T new_lower = std::min(lower_, other.lower_);
        const T new_upper = std::max(upper_, other.upper_);

        const bool new_left_open = (lower_ == other.lower_)
            ? (left_open_ && other.left_open_)
            : (new_lower == lower_ ? left_open_ : other.left_open_);

        const bool new_right_open = (upper_ == other.upper_)
            ? (right_open_ && other.right_open_)
            : (new_upper == upper_ ? right_open_ : other.right_open_);

        return interval(new_lower, new_upper, new_left_open, new_right_open);
    }

    // Predicates
    [[nodiscard]] constexpr bool subset_of(const interval& other) const noexcept {
        if (is_empty()) return true;
        if (other.is_empty()) return false;

        const bool left_ok = (other.lower_ < lower_) ||
            (other.lower_ == lower_ && (!other.left_open_ || left_open_));
        const bool right_ok = (other.upper_ > upper_) ||
            (other.upper_ == upper_ && (!other.right_open_ || right_open_));

        return left_ok && right_ok;
    }

    [[nodiscard]] constexpr bool superset_of(const interval& other) const noexcept {
        return other.subset_of(*this);
    }

    [[nodiscard]] constexpr bool overlaps(const interval& other) const noexcept {
        return !intersect(other).is_empty();
    }

    [[nodiscard]] constexpr bool adjacent(const interval& other) const noexcept {
        if (is_empty() || other.is_empty()) return false;

        if (upper_ == other.lower_) {
            return right_open_ != other.left_open_;
        }
        if (lower_ == other.upper_) {
            return left_open_ != other.right_open_;
        }

        // For discrete domains, check true adjacency
        if constexpr (boundary_traits_type::is_discrete()) {
            if (auto next = boundary_traits_type::next(upper_); next) {
                if (*next == other.lower_ && !right_open_ && !other.left_open_) {
                    return true;
                }
            }
            if (auto prev = boundary_traits_type::prev(lower_); prev) {
                if (*prev == other.upper_ && !left_open_ && !other.right_open_) {
                    return true;
                }
            }
        }

        return false;
    }

    [[nodiscard]] constexpr bool disjoint(const interval& other) const noexcept {
        return !overlaps(other);
    }

    // Three-way comparison (C++20)
    [[nodiscard]] constexpr auto operator<=>(const interval& other) const noexcept
        -> std::partial_ordering {
        if (is_empty() && other.is_empty()) return std::partial_ordering::equivalent;
        if (is_empty()) return std::partial_ordering::less;
        if (other.is_empty()) return std::partial_ordering::greater;

        if (auto cmp = lower_ <=> other.lower_; cmp != 0) return cmp;
        if (left_open_ < other.left_open_) return std::partial_ordering::less;
        if (left_open_ > other.left_open_) return std::partial_ordering::greater;
        if (auto cmp = upper_ <=> other.upper_; cmp != 0) return cmp;
        if (right_open_ < other.right_open_) return std::partial_ordering::less;
        if (right_open_ > other.right_open_) return std::partial_ordering::greater;
        return std::partial_ordering::equivalent;
    }

    [[nodiscard]] constexpr bool operator==(const interval& other) const noexcept {
        return (*this <=> other) == 0;
    }

    // Operator overloads (maintaining mathematical notation)
    [[nodiscard]] constexpr interval operator*(const interval& other) const noexcept {
        return intersect(other);
    }

    [[nodiscard]] constexpr interval operator&(const interval& other) const noexcept {
        return intersect(other);
    }

    // Builder pattern for fluent construction
    class builder {
    public:
        builder& from(T value) noexcept {
            lower_ = value;
            return *this;
        }

        builder& to(T value) noexcept {
            upper_ = value;
            return *this;
        }

        builder& open_left() noexcept {
            left_open_ = true;
            return *this;
        }

        builder& open_right() noexcept {
            right_open_ = true;
            return *this;
        }

        builder& closed_left() noexcept {
            left_open_ = false;
            return *this;
        }

        builder& closed_right() noexcept {
            right_open_ = false;
            return *this;
        }

        builder& open() noexcept {
            left_open_ = right_open_ = true;
            return *this;
        }

        builder& closed() noexcept {
            left_open_ = right_open_ = false;
            return *this;
        }

        [[nodiscard]] interval build() const noexcept {
            return interval(lower_, upper_, left_open_, right_open_);
        }

        [[nodiscard]] operator interval() const noexcept {
            return build();
        }

    private:
        T lower_{};
        T upper_{};
        bool left_open_ = false;
        bool right_open_ = false;
    };

    [[nodiscard]] static constexpr builder make() noexcept {
        return builder{};
    }

private:
    T lower_;
    T upper_;
    bool left_open_;
    bool right_open_;

    // Friend functions for ADL
    friend constexpr std::optional<T> infimum(const interval& i) noexcept {
        return i.lower();
    }

    friend constexpr std::optional<T> supremum(const interval& i) noexcept {
        return i.upper();
    }

    friend constexpr bool adjacent(const interval& a, const interval& b) noexcept {
        return a.adjacent(b);
    }

    friend constexpr bool disjoint(const interval& a, const interval& b) noexcept {
        return a.disjoint(b);
    }

    friend constexpr bool overlaps(const interval& a, const interval& b) noexcept {
        return a.overlaps(b);
    }
};

// Free function operators for subset relations
template<BoundaryType T>
[[nodiscard]] constexpr bool operator<(const interval<T>& lhs, const interval<T>& rhs) noexcept {
    return lhs.subset_of(rhs) && lhs != rhs;
}

template<BoundaryType T>
[[nodiscard]] constexpr bool operator<=(const interval<T>& lhs, const interval<T>& rhs) noexcept {
    return lhs.subset_of(rhs);
}

template<BoundaryType T>
[[nodiscard]] constexpr bool operator>(const interval<T>& lhs, const interval<T>& rhs) noexcept {
    return lhs.superset_of(rhs) && lhs != rhs;
}

template<BoundaryType T>
[[nodiscard]] constexpr bool operator>=(const interval<T>& lhs, const interval<T>& rhs) noexcept {
    return lhs.superset_of(rhs);
}

// Type aliases for common use cases
using real_interval = interval<double>;
using integer_interval = interval<int>;
using long_interval = interval<long>;

} // namespace disjoint_interval_set