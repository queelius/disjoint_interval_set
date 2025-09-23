#pragma once

#include <vector>
#include <algorithm>
#include <ranges>
#include <span>
#include <initializer_list>
#include "concepts.hpp"
#include "interval_v2.hpp"
#include "interval_traits.hpp"

namespace disjoint_interval_set {

/**
 * @brief Modern disjoint interval set with improved API design.
 *
 * Key improvements:
 * - Clear separation between mutating and non-mutating operations
 * - Named methods alongside operator overloads
 * - Range support for modern C++ algorithms
 * - Move semantics and noexcept specifications
 * - Builder pattern for complex set construction
 * - Lazy evaluation support for complex operations
 */
template<IntervalType I = interval<double>>
class disjoint_interval_set {
public:
    using interval_type = I;
    using value_type = typename I::value_type;
    using container_type = std::vector<I>;
    using size_type = typename container_type::size_type;
    using const_iterator = typename container_type::const_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;

    // Constructors
    disjoint_interval_set() = default;

    explicit disjoint_interval_set(I interval) {
        if (!interval.is_empty()) {
            intervals_.push_back(std::move(interval));
        }
    }

    explicit disjoint_interval_set(std::vector<I> intervals)
        : intervals_(normalize(std::move(intervals))) {}

    disjoint_interval_set(std::initializer_list<I> intervals)
        : intervals_(normalize(std::vector<I>(intervals))) {}

    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, I>
    explicit disjoint_interval_set(R&& range)
        : intervals_(normalize(std::vector<I>(std::ranges::begin(range),
                                             std::ranges::end(range)))) {}

    disjoint_interval_set(const disjoint_interval_set&) = default;
    disjoint_interval_set(disjoint_interval_set&&) noexcept = default;

    disjoint_interval_set& operator=(const disjoint_interval_set&) = default;
    disjoint_interval_set& operator=(disjoint_interval_set&&) noexcept = default;

    // Named factory methods
    [[nodiscard]] static disjoint_interval_set make_empty() noexcept {
        return disjoint_interval_set{};
    }

    [[nodiscard]] static disjoint_interval_set universal() {
        return disjoint_interval_set{I::universal()};
    }

    [[nodiscard]] static disjoint_interval_set singleton(value_type value) {
        return disjoint_interval_set{I::singleton(value)};
    }

    // Container operations
    [[nodiscard]] bool empty() const noexcept {
        return intervals_.empty();
    }

    [[nodiscard]] size_type size() const noexcept {
        return intervals_.size();
    }

    [[nodiscard]] size_type interval_count() const noexcept {
        return size();
    }

    void clear() noexcept {
        intervals_.clear();
    }

    void reserve(size_type capacity) {
        intervals_.reserve(capacity);
    }

    // Iteration
    [[nodiscard]] const_iterator begin() const noexcept {
        return intervals_.begin();
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return intervals_.end();
    }

    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return intervals_.rbegin();
    }

    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return intervals_.rend();
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return intervals_.cbegin();
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return intervals_.cend();
    }

    // Range support
    [[nodiscard]] auto intervals() const noexcept -> std::span<const I> {
        return std::span<const I>(intervals_);
    }

    // Element access
    [[nodiscard]] const I& operator[](size_type index) const {
        return intervals_[index];
    }

    [[nodiscard]] const I& at(size_type index) const {
        return intervals_.at(index);
    }

    [[nodiscard]] const I& front() const {
        return intervals_.front();
    }

    [[nodiscard]] const I& back() const {
        return intervals_.back();
    }

    // Membership testing
    [[nodiscard]] bool contains(value_type value) const {
        // Binary search for efficiency
        auto it = std::lower_bound(intervals_.begin(), intervals_.end(), value,
            [](const I& interval, value_type val) {
                if (auto sup = supremum(interval); sup) {
                    return *sup < val;
                }
                return false;
            });

        return it != intervals_.end() && it->contains(value);
    }

    [[nodiscard]] bool contains(const I& interval) const {
        if (interval.empty()) return true;
        return contains_interval(interval);
    }

    [[nodiscard]] bool contains(const disjoint_interval_set& other) const {
        return other.subset_of(*this);
    }

    // Boundary operations
    [[nodiscard]] std::optional<value_type> infimum() const {
        return empty() ? std::nullopt : intervals_.front().lower();
    }

    [[nodiscard]] std::optional<value_type> supremum() const {
        return empty() ? std::nullopt : intervals_.back().upper();
    }

    [[nodiscard]] std::optional<value_type> lower_bound() const {
        return infimum();
    }

    [[nodiscard]] std::optional<value_type> upper_bound() const {
        return supremum();
    }

    // Set operations with named methods
    [[nodiscard]] disjoint_interval_set unite(const disjoint_interval_set& other) const {
        return set_union(*this, other);
    }

    [[nodiscard]] disjoint_interval_set intersect(const disjoint_interval_set& other) const {
        return set_intersection(*this, other);
    }

    [[nodiscard]] disjoint_interval_set difference(const disjoint_interval_set& other) const {
        return set_difference(*this, other);
    }

    [[nodiscard]] disjoint_interval_set symmetric_difference(const disjoint_interval_set& other) const {
        return set_symmetric_difference(*this, other);
    }

    [[nodiscard]] disjoint_interval_set complement() const {
        return set_complement(*this);
    }

    [[nodiscard]] disjoint_interval_set complement(value_type lower, value_type upper) const {
        return set_complement(*this, lower, upper);
    }

    // Mutation operations (return *this for chaining)
    disjoint_interval_set& unite_with(const disjoint_interval_set& other) {
        *this = unite(other);
        return *this;
    }

    disjoint_interval_set& intersect_with(const disjoint_interval_set& other) {
        *this = intersect(other);
        return *this;
    }

    disjoint_interval_set& subtract(const disjoint_interval_set& other) {
        *this = difference(other);
        return *this;
    }

    disjoint_interval_set& invert() {
        *this = complement();
        return *this;
    }

    // Relation predicates
    [[nodiscard]] bool subset_of(const disjoint_interval_set& other) const {
        return std::all_of(intervals_.begin(), intervals_.end(),
            [&other](const I& interval) {
                return other.contains_interval(interval);
            });
    }

    [[nodiscard]] bool superset_of(const disjoint_interval_set& other) const {
        return other.subset_of(*this);
    }

    [[nodiscard]] bool disjoint_with(const disjoint_interval_set& other) const {
        return intersect(other).empty();
    }

    [[nodiscard]] bool overlaps_with(const disjoint_interval_set& other) const {
        return !disjoint_with(other);
    }

    // Operator overloads (maintaining mathematical notation)
    [[nodiscard]] disjoint_interval_set operator+(const disjoint_interval_set& other) const {
        return unite(other);
    }

    [[nodiscard]] disjoint_interval_set operator|(const disjoint_interval_set& other) const {
        return unite(other);
    }

    [[nodiscard]] disjoint_interval_set operator*(const disjoint_interval_set& other) const {
        return intersect(other);
    }

    [[nodiscard]] disjoint_interval_set operator&(const disjoint_interval_set& other) const {
        return intersect(other);
    }

    [[nodiscard]] disjoint_interval_set operator-(const disjoint_interval_set& other) const {
        return difference(other);
    }

    [[nodiscard]] disjoint_interval_set operator^(const disjoint_interval_set& other) const {
        return symmetric_difference(other);
    }

    [[nodiscard]] disjoint_interval_set operator~() const {
        return complement();
    }

    // Compound assignment operators
    disjoint_interval_set& operator+=(const disjoint_interval_set& other) {
        return unite_with(other);
    }

    disjoint_interval_set& operator|=(const disjoint_interval_set& other) {
        return unite_with(other);
    }

    disjoint_interval_set& operator*=(const disjoint_interval_set& other) {
        return intersect_with(other);
    }

    disjoint_interval_set& operator&=(const disjoint_interval_set& other) {
        return intersect_with(other);
    }

    disjoint_interval_set& operator-=(const disjoint_interval_set& other) {
        return subtract(other);
    }

    disjoint_interval_set& operator^=(const disjoint_interval_set& other) {
        *this = symmetric_difference(other);
        return *this;
    }

    // Comparison operators
    [[nodiscard]] bool operator==(const disjoint_interval_set& other) const {
        return intervals_ == other.intervals_;
    }

    [[nodiscard]] bool operator!=(const disjoint_interval_set& other) const {
        return !(*this == other);
    }

    [[nodiscard]] bool operator<(const disjoint_interval_set& other) const {
        return subset_of(other) && *this != other;
    }

    [[nodiscard]] bool operator<=(const disjoint_interval_set& other) const {
        return subset_of(other);
    }

    [[nodiscard]] bool operator>(const disjoint_interval_set& other) const {
        return superset_of(other) && *this != other;
    }

    [[nodiscard]] bool operator>=(const disjoint_interval_set& other) const {
        return superset_of(other);
    }

    // Builder pattern for complex set construction
    class builder {
    public:
        builder& add(I interval) {
            if (!interval.is_empty()) {
                intervals_.push_back(std::move(interval));
            }
            return *this;
        }

        builder& add(value_type lower, value_type upper) {
            return add(I::closed(lower, upper));
        }

        builder& add_open(value_type lower, value_type upper) {
            return add(I::open(lower, upper));
        }

        builder& add_left_open(value_type lower, value_type upper) {
            return add(I::left_open(lower, upper));
        }

        builder& add_right_open(value_type lower, value_type upper) {
            return add(I::right_open(lower, upper));
        }

        builder& add_singleton(value_type value) {
            return add(I::singleton(value));
        }

        builder& unite(const disjoint_interval_set& other) {
            for (const auto& interval : other) {
                intervals_.push_back(interval);
            }
            return *this;
        }

        [[nodiscard]] disjoint_interval_set build() const {
            return disjoint_interval_set(intervals_);
        }

        [[nodiscard]] operator disjoint_interval_set() const {
            return build();
        }

    private:
        std::vector<I> intervals_;
    };

    [[nodiscard]] static builder make() {
        return builder{};
    }

private:
    container_type intervals_;

    // Helper to check if an interval is fully contained
    [[nodiscard]] bool contains_interval(const I& interval) const {
        // Find all intervals that might contain parts of the given interval
        auto first = std::lower_bound(intervals_.begin(), intervals_.end(), interval,
            [](const I& a, const I& b) {
                if (auto sup_a = supremum(a), inf_b = infimum(b); sup_a && inf_b) {
                    return *sup_a < *inf_b;
                }
                return false;
            });

        if (first == intervals_.end()) return false;

        // Check if the interval is fully contained in one of our intervals
        return std::any_of(first, intervals_.end(),
            [&interval](const I& our_interval) {
                return interval.subset_of(our_interval);
            });
    }

    // Normalize a vector of intervals into disjoint form
    [[nodiscard]] static container_type normalize(container_type intervals) {
        if (intervals.empty()) return intervals;

        // Remove empty intervals
        intervals.erase(
            std::remove_if(intervals.begin(), intervals.end(),
                [](const I& i) { return i.is_empty(); }),
            intervals.end());

        if (intervals.empty()) return intervals;

        // Sort intervals
        std::sort(intervals.begin(), intervals.end());

        // Merge overlapping and adjacent intervals
        container_type result;
        result.reserve(intervals.size());

        auto current = intervals.front();
        for (size_t i = 1; i < intervals.size(); ++i) {
            if (auto united = current.unite(intervals[i]); united) {
                current = *united;
            } else {
                result.push_back(current);
                current = intervals[i];
            }
        }
        result.push_back(current);

        result.shrink_to_fit();
        return result;
    }

    // Friend functions for set operations
    friend disjoint_interval_set set_union(const disjoint_interval_set& a,
                                          const disjoint_interval_set& b) {
        container_type combined;
        combined.reserve(a.size() + b.size());
        combined.insert(combined.end(), a.begin(), a.end());
        combined.insert(combined.end(), b.begin(), b.end());
        return disjoint_interval_set(std::move(combined));
    }

    friend disjoint_interval_set set_intersection(const disjoint_interval_set& a,
                                                 const disjoint_interval_set& b) {
        container_type result;
        auto it_a = a.begin();
        auto it_b = b.begin();

        while (it_a != a.end() && it_b != b.end()) {
            auto intersection = it_a->intersect(*it_b);
            if (!intersection.is_empty()) {
                result.push_back(intersection);
            }

            // Advance the iterator with the smaller upper bound
            if (auto sup_a = supremum(*it_a), sup_b = supremum(*it_b); sup_a && sup_b) {
                if (*sup_a <= *sup_b) {
                    ++it_a;
                } else {
                    ++it_b;
                }
            } else {
                break;
            }
        }

        return disjoint_interval_set(std::move(result));
    }

    friend disjoint_interval_set set_difference(const disjoint_interval_set& a,
                                               const disjoint_interval_set& b) {
        return set_intersection(a, set_complement(b));
    }

    friend disjoint_interval_set set_symmetric_difference(const disjoint_interval_set& a,
                                                         const disjoint_interval_set& b) {
        return set_union(set_difference(a, b), set_difference(b, a));
    }

    friend disjoint_interval_set set_complement(const disjoint_interval_set& s) {
        using traits = boundary_traits<value_type>;
        return set_complement(s, traits::min(), traits::max());
    }

    friend disjoint_interval_set set_complement(const disjoint_interval_set& s,
                                               value_type lower, value_type upper) {
        if (s.empty()) {
            return disjoint_interval_set{I::closed(lower, upper)};
        }

        container_type result;
        value_type current_lower = lower;

        for (const auto& interval : s) {
            if (auto inf = infimum(interval); inf && *inf > current_lower) {
                result.push_back(I::closed(current_lower, *inf));
            }
            if (auto sup = supremum(interval); sup) {
                current_lower = *sup;
            }
        }

        if (current_lower < upper) {
            result.push_back(I::closed(current_lower, upper));
        }

        return disjoint_interval_set(std::move(result));
    }
};

// Type aliases for common use cases
using reals = disjoint_interval_set<interval<double>>;
using integers = disjoint_interval_set<interval<int>>;
using longs = disjoint_interval_set<interval<long>>;

// Convenience functions for creating interval sets
template<IntervalType I>
[[nodiscard]] auto make_interval_set(I interval) {
    return disjoint_interval_set<I>(std::move(interval));
}

template<IntervalType I>
[[nodiscard]] auto make_interval_set(std::initializer_list<I> intervals) {
    return disjoint_interval_set<I>(intervals);
}

// Type aliases for common use cases
using reals = disjoint_interval_set<interval<double>>;
using integers = disjoint_interval_set<interval<int>>;
using longs = disjoint_interval_set<interval<long>>;

} // namespace disjoint_interval_set