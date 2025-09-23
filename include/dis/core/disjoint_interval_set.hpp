#pragma once

#include "interval.hpp"
#include <vector>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <span>
#include <initializer_list>
#include <functional>

namespace dis {

/**
 * @brief A set of disjoint intervals forming a Boolean algebra.
 *
 * This class maintains a normalized collection of non-overlapping intervals,
 * providing efficient set-theoretic operations and queries.
 *
 * Design principles:
 * - Automatically maintains disjoint invariant
 * - Fluent/chainable interface for natural expression
 * - Value semantics with move optimization
 * - Lazy evaluation where beneficial
 * - Clear separation of queries and mutations
 */
template<typename I>
class disjoint_interval_set {
public:
    using interval_type = I;
    using value_type = typename I::value_type;
    using container_type = std::vector<I>;
    using size_type = std::size_t;
    using const_iterator = typename container_type::const_iterator;

    // === Construction ===

    disjoint_interval_set() = default;

    explicit disjoint_interval_set(I interval) {
        if (!interval.is_empty()) {
            intervals_.push_back(std::move(interval));
        }
    }

    disjoint_interval_set(std::initializer_list<I> intervals) {
        for (const auto& interval : intervals) {
            if (!interval.is_empty()) {
                intervals_.push_back(interval);
            }
        }
        normalize();
    }

    template<std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, I>
    explicit disjoint_interval_set(R&& range) {
        for (const auto& interval : range) {
            if (!interval.is_empty()) {
                intervals_.push_back(interval);
            }
        }
        normalize();
    }

    // === Named Constructors ===

    [[nodiscard]] static disjoint_interval_set empty() {
        return disjoint_interval_set{};
    }

    [[nodiscard]] static disjoint_interval_set point(value_type value) {
        return disjoint_interval_set{I::point(value)};
    }

    [[nodiscard]] static disjoint_interval_set unbounded()
        requires std::numeric_limits<value_type>::has_infinity {
        return disjoint_interval_set{I::unbounded()};
    }

    [[nodiscard]] static disjoint_interval_set from_string(std::string_view str);
    // Defined in io/parser.hpp

    // === Core Queries ===

    [[nodiscard]] bool is_empty() const noexcept {
        return intervals_.empty();
    }

    [[nodiscard]] size_type size() const noexcept {
        return intervals_.size();
    }

    [[nodiscard]] bool contains(value_type value) const {
        // Binary search for efficiency
        auto it = std::lower_bound(intervals_.begin(), intervals_.end(), value,
            [](const I& interval, value_type v) {
                return interval.upper_bound().value_or(v) < v;
            });

        return it != intervals_.end() && it->contains(value);
    }

    [[nodiscard]] bool contains(const I& interval) const {
        if (interval.is_empty()) return true;

        for (const auto& i : intervals_) {
            if (interval.subset_of(i)) return true;
        }
        return false;
    }

    // === DIS-Specific Queries ===

    [[nodiscard]] I span() const {
        if (is_empty()) return I::empty();
        return I::closed(
            *intervals_.front().lower_bound(),
            *intervals_.back().upper_bound()
        );
    }

    [[nodiscard]] disjoint_interval_set gaps() const {
        if (size() <= 1) return disjoint_interval_set{};

        disjoint_interval_set result;
        for (size_t i = 0; i < size() - 1; ++i) {
            auto upper = intervals_[i].upper_bound().value();
            auto lower = intervals_[i + 1].lower_bound().value();

            bool left_closed = !intervals_[i].is_right_closed();
            bool right_closed = !intervals_[i + 1].is_left_closed();

            result.intervals_.push_back(I(upper, lower, left_closed, right_closed));
        }
        return result;
    }

    [[nodiscard]] size_type component_count() const noexcept {
        return size();
    }

    [[nodiscard]] std::vector<I> components() const {
        return intervals_;
    }

    [[nodiscard]] value_type measure() const
        requires std::is_arithmetic_v<value_type> {
        return std::accumulate(intervals_.begin(), intervals_.end(), value_type(0),
            [](value_type sum, const I& interval) {
                return sum + interval.length();
            });
    }

    [[nodiscard]] value_type gap_measure() const
        requires std::is_arithmetic_v<value_type> {
        return gaps().measure();
    }

    [[nodiscard]] double density() const
        requires std::is_arithmetic_v<value_type> {
        auto s = span();
        if (s.is_empty()) return 0.0;
        return static_cast<double>(measure()) / static_cast<double>(s.length());
    }

    // === Iteration ===

    [[nodiscard]] const_iterator begin() const noexcept { return intervals_.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return intervals_.end(); }

    [[nodiscard]] const I& operator[](size_type index) const { return intervals_[index]; }
    [[nodiscard]] const I& at(size_type index) const { return intervals_.at(index); }

    [[nodiscard]] std::span<const I> intervals() const noexcept {
        return std::span<const I>(intervals_);
    }

    // === Set Relations ===

    [[nodiscard]] bool subset_of(const disjoint_interval_set& other) const {
        for (const auto& interval : intervals_) {
            if (!other.contains(interval)) return false;
        }
        return true;
    }

    [[nodiscard]] bool superset_of(const disjoint_interval_set& other) const {
        return other.subset_of(*this);
    }

    [[nodiscard]] bool disjoint_from(const disjoint_interval_set& other) const {
        return intersect(other).is_empty();
    }

    [[nodiscard]] bool overlaps(const disjoint_interval_set& other) const {
        return !disjoint_from(other);
    }

    // === Set Operations (Immutable) ===

    [[nodiscard]] disjoint_interval_set unite(const disjoint_interval_set& other) const {
        if (is_empty()) return other;
        if (other.is_empty()) return *this;

        disjoint_interval_set result;
        result.intervals_.reserve(size() + other.size());

        result.intervals_.insert(result.intervals_.end(), intervals_.begin(), intervals_.end());
        result.intervals_.insert(result.intervals_.end(), other.intervals_.begin(), other.intervals_.end());

        result.normalize();
        return result;
    }

    [[nodiscard]] disjoint_interval_set intersect(const disjoint_interval_set& other) const {
        if (is_empty() || other.is_empty()) return disjoint_interval_set{};

        disjoint_interval_set result;

        for (const auto& a : intervals_) {
            for (const auto& b : other.intervals_) {
                auto intersection = a.intersect(b);
                if (!intersection.is_empty()) {
                    result.intervals_.push_back(intersection);
                }
            }
        }

        result.normalize();
        return result;
    }

    [[nodiscard]] disjoint_interval_set complement() const
        requires std::numeric_limits<value_type>::has_infinity {
        if (is_empty()) return unbounded();

        disjoint_interval_set result;
        value_type neg_inf = -std::numeric_limits<value_type>::infinity();
        value_type pos_inf = std::numeric_limits<value_type>::infinity();

        // Before first interval
        auto first_lower = intervals_.front().lower_bound().value();
        if (first_lower != neg_inf) {
            bool right_closed = !intervals_.front().is_left_closed();
            result.intervals_.push_back(I(neg_inf, first_lower, false, right_closed));
        }

        // Between intervals
        for (size_t i = 0; i < size() - 1; ++i) {
            auto upper = intervals_[i].upper_bound().value();
            auto lower = intervals_[i + 1].lower_bound().value();
            bool left_closed = !intervals_[i].is_right_closed();
            bool right_closed = !intervals_[i + 1].is_left_closed();
            result.intervals_.push_back(I(upper, lower, left_closed, right_closed));
        }

        // After last interval
        auto last_upper = intervals_.back().upper_bound().value();
        if (last_upper != pos_inf) {
            bool left_closed = !intervals_.back().is_right_closed();
            result.intervals_.push_back(I(last_upper, pos_inf, left_closed, false));
        }

        return result;
    }

    [[nodiscard]] disjoint_interval_set difference(const disjoint_interval_set& other) const
        requires std::numeric_limits<value_type>::has_infinity {
        return intersect(other.complement());
    }

    [[nodiscard]] disjoint_interval_set symmetric_difference(const disjoint_interval_set& other) const
        requires std::numeric_limits<value_type>::has_infinity {
        return unite(other).difference(intersect(other));
    }

    // === Fluent Interface (Chainable Operations) ===

    [[nodiscard]] disjoint_interval_set& add(I interval) & {
        if (!interval.is_empty()) {
            intervals_.push_back(std::move(interval));
            normalize();
        }
        return *this;
    }

    [[nodiscard]] disjoint_interval_set&& add(I interval) && {
        return std::move(add(interval));
    }

    [[nodiscard]] disjoint_interval_set& add(value_type lower, value_type upper) & {
        return add(I::closed(lower, upper));
    }

    [[nodiscard]] disjoint_interval_set&& add(value_type lower, value_type upper) && {
        return std::move(add(lower, upper));
    }

    [[nodiscard]] disjoint_interval_set& remove(I interval) &
        requires std::numeric_limits<value_type>::has_infinity {
        *this = difference(disjoint_interval_set{interval});
        return *this;
    }

    [[nodiscard]] disjoint_interval_set&& remove(I interval) &&
        requires std::numeric_limits<value_type>::has_infinity {
        return std::move(remove(interval));
    }

    [[nodiscard]] disjoint_interval_set& coalesce() & {
        normalize();
        return *this;
    }

    [[nodiscard]] disjoint_interval_set&& coalesce() && {
        return std::move(coalesce());
    }

    // === Functional Operations ===

    template<typename F>
    [[nodiscard]] disjoint_interval_set filter(F&& predicate) const {
        disjoint_interval_set result;
        for (const auto& interval : intervals_) {
            if (std::invoke(std::forward<F>(predicate), interval)) {
                result.intervals_.push_back(interval);
            }
        }
        return result;
    }

    template<typename F>
    [[nodiscard]] auto map(F&& transform) const {
        using result_type = std::invoke_result_t<F, I>;
        disjoint_interval_set<result_type> result;
        for (const auto& interval : intervals_) {
            auto transformed = std::invoke(std::forward<F>(transform), interval);
            result = result.add(transformed);
        }
        return result;
    }

    template<typename F>
    void for_each(F&& action) const {
        for (const auto& interval : intervals_) {
            std::invoke(std::forward<F>(action), interval);
        }
    }

    // === Comparison Operators ===

    [[nodiscard]] friend bool operator==(const disjoint_interval_set& a,
                                         const disjoint_interval_set& b) {
        return a.intervals_ == b.intervals_;
    }

    [[nodiscard]] friend auto operator<=>(const disjoint_interval_set& a,
                                          const disjoint_interval_set& b) {
        return a.intervals_ <=> b.intervals_;
    }

    // === Mathematical Notation Operators ===

    [[nodiscard]] friend disjoint_interval_set operator|(const disjoint_interval_set& a,
                                                        const disjoint_interval_set& b) {
        return a.unite(b);
    }

    [[nodiscard]] friend disjoint_interval_set operator&(const disjoint_interval_set& a,
                                                        const disjoint_interval_set& b) {
        return a.intersect(b);
    }

    [[nodiscard]] friend disjoint_interval_set operator-(const disjoint_interval_set& a,
                                                        const disjoint_interval_set& b)
        requires std::numeric_limits<value_type>::has_infinity {
        return a.difference(b);
    }

    [[nodiscard]] friend disjoint_interval_set operator^(const disjoint_interval_set& a,
                                                        const disjoint_interval_set& b)
        requires std::numeric_limits<value_type>::has_infinity {
        return a.symmetric_difference(b);
    }

    [[nodiscard]] friend disjoint_interval_set operator~(const disjoint_interval_set& a)
        requires std::numeric_limits<value_type>::has_infinity {
        return a.complement();
    }

    // Compound assignment operators
    disjoint_interval_set& operator|=(const disjoint_interval_set& other) {
        *this = unite(other);
        return *this;
    }

    disjoint_interval_set& operator&=(const disjoint_interval_set& other) {
        *this = intersect(other);
        return *this;
    }

    disjoint_interval_set& operator-=(const disjoint_interval_set& other)
        requires std::numeric_limits<value_type>::has_infinity {
        *this = difference(other);
        return *this;
    }

    disjoint_interval_set& operator^=(const disjoint_interval_set& other)
        requires std::numeric_limits<value_type>::has_infinity {
        *this = symmetric_difference(other);
        return *this;
    }

private:
    container_type intervals_;

    // Normalize intervals to maintain disjoint invariant
    void normalize() {
        if (intervals_.size() <= 1) return;

        // Sort intervals by lower bound
        std::ranges::sort(intervals_);

        // Merge overlapping and adjacent intervals
        size_t write = 0;
        for (size_t read = 1; read < intervals_.size(); ++read) {
            auto merged = intervals_[write].hull(intervals_[read]);
            if (merged) {
                intervals_[write] = *merged;
            } else {
                intervals_[++write] = intervals_[read];
            }
        }
        intervals_.resize(write + 1);
    }
};

// Type aliases for common use cases
using real_set = disjoint_interval_set<real_interval>;
using integer_set = disjoint_interval_set<integer_interval>;

} // namespace dis