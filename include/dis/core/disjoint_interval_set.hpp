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
    // Standard container type aliases
    using interval_type = I;
    using value_type = typename I::value_type;
    using container_type = std::vector<I>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = const I&;
    using const_reference = const I&;
    using pointer = const I*;
    using const_pointer = const I*;
    using iterator = typename container_type::const_iterator;
    using const_iterator = typename container_type::const_iterator;
    using reverse_iterator = std::reverse_iterator<const_iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // === Construction ===

    // Explicitly defaulted special member functions (Rule of Five)
    disjoint_interval_set() = default;
    disjoint_interval_set(const disjoint_interval_set&) = default;
    disjoint_interval_set(disjoint_interval_set&&) noexcept = default;
    disjoint_interval_set& operator=(const disjoint_interval_set&) = default;
    disjoint_interval_set& operator=(disjoint_interval_set&&) noexcept = default;
    ~disjoint_interval_set() = default;

    explicit disjoint_interval_set(I interval) {
        if (!interval.empty()) {
            intervals_.push_back(std::move(interval));
        }
    }

    disjoint_interval_set(std::initializer_list<I> intervals) {
        for (const auto& interval : intervals) {
            if (!interval.empty()) {
                intervals_.push_back(interval);
            }
        }
        normalize();
    }

    template<std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, I>
    explicit disjoint_interval_set(R&& range) {
        for (const auto& interval : range) {
            if (!interval.empty()) {
                intervals_.push_back(interval);
            }
        }
        normalize();
    }

    // === Named Constructors ===

    // Note: No static empty() factory - use default constructor disjoint_interval_set()
    // This avoids name collision with the STL-standard empty() query method

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

    [[nodiscard]] bool empty() const noexcept {
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
        if (interval.empty()) return true;

        for (const auto& i : intervals_) {
            if (interval.subset_of(i)) return true;
        }
        return false;
    }

    // === DIS-Specific Queries ===

    [[nodiscard]] I span() const {
        if (empty()) return I{};
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
        if (s.empty()) return 0.0;
        return static_cast<double>(measure()) / static_cast<double>(s.length());
    }

    // === Iteration (STL-compatible) ===

    [[nodiscard]] const_iterator begin() const noexcept { return intervals_.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return intervals_.end(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return intervals_.cbegin(); }
    [[nodiscard]] const_iterator cend() const noexcept { return intervals_.cend(); }

    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(intervals_.end());
    }
    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(intervals_.begin());
    }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(intervals_.cend());
    }
    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(intervals_.cbegin());
    }

    // === Element Access ===

    [[nodiscard]] const I& operator[](size_type index) const { return intervals_[index]; }
    [[nodiscard]] const I& at(size_type index) const { return intervals_.at(index); }

    [[nodiscard]] const I& front() const { return intervals_.front(); }
    [[nodiscard]] const I& back() const { return intervals_.back(); }

    [[nodiscard]] std::span<const I> intervals() const noexcept {
        return std::span<const I>(intervals_);
    }

    // === Capacity ===

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return intervals_.max_size();
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
        return intersect(other).empty();
    }

    [[nodiscard]] bool overlaps(const disjoint_interval_set& other) const {
        return !disjoint_from(other);
    }

    // === Set Operations (Immutable) ===

    [[nodiscard]] disjoint_interval_set unite(const disjoint_interval_set& other) const {
        if (empty()) return other;
        if (other.empty()) return *this;

        disjoint_interval_set result;
        result.intervals_.reserve(size() + other.size());

        result.intervals_.insert(result.intervals_.end(), intervals_.begin(), intervals_.end());
        result.intervals_.insert(result.intervals_.end(), other.intervals_.begin(), other.intervals_.end());

        result.normalize();
        return result;
    }

    [[nodiscard]] disjoint_interval_set intersect(const disjoint_interval_set& other) const {
        if (empty() || other.empty()) return disjoint_interval_set{};

        disjoint_interval_set result;

        for (const auto& a : intervals_) {
            for (const auto& b : other.intervals_) {
                auto intersection = a.intersect(b);
                if (!intersection.empty()) {
                    result.intervals_.push_back(intersection);
                }
            }
        }

        result.normalize();
        return result;
    }

    [[nodiscard]] disjoint_interval_set complement() const
        requires std::numeric_limits<value_type>::has_infinity {
        if (empty()) return unbounded();

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

    // === Modifiers (STL-compatible) ===

    // Clear all intervals
    void clear() noexcept {
        intervals_.clear();
    }

    // Swap with another set
    void swap(disjoint_interval_set& other) noexcept {
        intervals_.swap(other.intervals_);
    }

    // Insert interval with STL-compatible return type
    // Returns iterator to the first interval affected/created
    iterator insert(I interval) {
        if (interval.empty()) {
            return end();
        }

        intervals_.push_back(std::move(interval));
        normalize();

        // Find and return iterator to one of the intervals containing the inserted data
        // Due to normalization, the inserted interval may have merged with others
        return begin();  // Conservative: always return begin after normalization
    }

    // Insert range of intervals
    template<std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, I>
    void insert(R&& range) {
        for (const auto& interval : range) {
            if (!interval.empty()) {
                intervals_.push_back(interval);
            }
        }
        normalize();
    }

    // Insert iterator range
    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            if (!it->empty()) {
                intervals_.push_back(*it);
            }
        }
        normalize();
    }

    // Insert initializer list
    void insert(std::initializer_list<I> ilist) {
        for (const auto& interval : ilist) {
            if (!interval.empty()) {
                intervals_.push_back(interval);
            }
        }
        normalize();
    }

    // Erase interval at iterator position
    iterator erase(const_iterator pos) {
        return intervals_.erase(pos);
    }

    // Erase range of intervals
    iterator erase(const_iterator first, const_iterator last) {
        return intervals_.erase(first, last);
    }

    // Erase intervals matching the given interval (returns count)
    size_type erase(const I& interval) {
        auto it = std::find(intervals_.begin(), intervals_.end(), interval);
        if (it != intervals_.end()) {
            intervals_.erase(it);
            return 1;
        }
        return 0;
    }

    // Set subtraction (remove intervals covered by another set)
    // This is separate from erase() which removes by exact match
    disjoint_interval_set& subtract(const disjoint_interval_set& other)
        requires std::numeric_limits<value_type>::has_infinity {
        *this = difference(other);
        return *this;
    }

    // === Fluent Interface (Chainable Operations) ===
    // Deprecated: Use insert() for STL compatibility
    // These are kept for backward compatibility and chaining style

    [[nodiscard]] disjoint_interval_set& add(I interval) & {
        insert(std::move(interval));
        return *this;
    }

    [[nodiscard]] disjoint_interval_set&& add(I interval) && {
        insert(std::move(interval));
        return std::move(*this);
    }

    [[nodiscard]] disjoint_interval_set& add(value_type lower, value_type upper) & {
        insert(I::closed(lower, upper));
        return *this;
    }

    [[nodiscard]] disjoint_interval_set&& add(value_type lower, value_type upper) && {
        insert(I::closed(lower, upper));
        return std::move(*this);
    }

    [[nodiscard]] disjoint_interval_set& remove(I interval) &
        requires std::numeric_limits<value_type>::has_infinity {
        subtract(disjoint_interval_set{interval});
        return *this;
    }

    [[nodiscard]] disjoint_interval_set&& remove(I interval) &&
        requires std::numeric_limits<value_type>::has_infinity {
        subtract(disjoint_interval_set{interval});
        return std::move(*this);
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

// === Free Functions (STL-compatible) ===

// ADL-friendly swap
template<typename I>
void swap(disjoint_interval_set<I>& a, disjoint_interval_set<I>& b) noexcept {
    a.swap(b);
}

} // namespace dis

// === C++20 Ranges Support ===

namespace std::ranges {
    template<typename I>
    inline constexpr bool enable_borrowed_range<dis::disjoint_interval_set<I>> = true;
}