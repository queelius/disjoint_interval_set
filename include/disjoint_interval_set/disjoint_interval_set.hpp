#pragma once

#include <vector>
#include "disjoint_interval_set_algorithms.hpp"
#include "interval.hpp"

namespace set::interval
{
    /**
     * Models the concept of a disjoint set of intervals. It is a Boolean
     * algebra over disjoint interval sets equipped with all the standard
     * set-theoretic operations, like intersection (*), union (+), and
     * complement (~).
     */
	template <typename I = interval<double>,
              typename A = std::allocator<I>,
              typename C = std::vector<I,A>>
	class disjoint_interval_set
	{
	public:
		using interval_type = I;
        using value_type = typename I::value_type;
		using const_iterator = typename C::const_iterator;
        using allocator = A;
        using container = C;

		// constuctors
		disjoint_interval_set() = default;
		disjoint_interval_set(const disjoint_interval_set &) = default;

		// accessors
		auto sup() const { return empty() ? std::nullopt : s_.back().max(); }
		auto inf() const { return empty() ? std::nullopt : s_.front().min(); }
		auto contains(value_type v) const { return any_of(begin(), end(), [v](I const & i) { return i.contains(v); }); }
		auto empty() const { return s_.empty(); }
		auto begin() const { return s_.begin(); }
		auto end() const { return s_.end(); }

	private:
		C s_;
	};

    using real_set = disjoint_interval_set<>;
    using integer_set = disjoint_interval_set<interval<int>>;

    /**
     * relation predicates
     */

    // subset predicate
    template <typename I,typename A,typename C>
    auto operator<=(
        disjoint_interval_set<I,A,C> const & lhs,
        disjoint_interval_set<I,A,C> const & rhs)
    {
        auto i = lhs.begin();
        auto j = rhs.begin();
        while (i != lhs.end() && j != rhs.end())
        {
            if (i->contains(*j))
                ++j;
            else
                ++i;
        }
        return j == rhs.end();
    }

    // superset predicate
    template <typename I,typename A,typename C>
    auto operator>=(
        disjoint_interval_set<I,A,C> const & lhs,
        disjoint_interval_set<I,A,C> const & rhs)
    {
        return rhs <= lhs;
    }

    // equality predicate
    template <typename I,typename A,typename C>
    auto operator==(
        disjoint_interval_set<I,A,C> const & lhs,
        disjoint_interval_set<I,A,C> const & rhs)
    {
        return (rhs <= lhs) && (lhs <= rhs);
    }

    // inequality predicate
    template <typename I,typename C>
    auto operator!=(
        disjoint_interval_set<I,C> const & lhs,
        disjoint_interval_set<I,C> const & rhs)
    {
        return !(lhs == rhs);
    }

    // proper subset predicate
    template <typename I,typename A,typename C>
    auto operator<(
        disjoint_interval_set<I,A,C> const & lhs,
        disjoint_interval_set<I,A,C> const & rhs)
    {
        return (lhs <= rhs) && (lhs != rhs);
    }

    // proper superset predicate
    template <typename I,typename C>
    auto operator>(
        disjoint_interval_set<I,C> const & lhs,
        disjoint_interval_set<I,C> const & rhs)
    {
        return (lhs >= lhs) && (lhs != rhs);
    }

    /**
     * set-theoretic operations
     */

    // intersection
    template <typename I,typename C>
    auto operator*(
        disjoint_interval_set<I,C> lhs,
        disjoint_interval_set<I,C> rhs)
    {
        return ~((~move(lhs)) + (~move(rhs)));
    }

    template <typename I,typename C>
    auto symmetric_difference(
        disjoint_interval_set<I,C> const & lhs,
        disjoint_interval_set<I,C> const & rhs)
    {
        return (lhs * (~rhs)) + (~(lhs) * rhs);
    }

    // complement
    template <typename T>
    auto operator~(disjoint_interval_set<T> x)
    {
        x.s_ = complement_disjoint_interval_set(move(x.s_));
        return x;
    }

    // set-difference
    template <typename T>
    auto operator-(
        disjoint_interval_set<T> lhs,
        disjoint_interval_set<T> rhs)
    {
        return move(lhs) * (~move(rhs));
    }

    // union
	template <typename I,typename C>
    auto operator+(
        disjoint_interval_set<I,C> const & lhs,
        disjoint_interval_set<I,C> rhs)
	{
		if (lhs.empty())
			return rhs;
		if (rhs.empty())
			return lhs;

		rhs.s_.insert(rhs.s_.end(), lhs.s_.begin(), lhs.s_.end());
		rhs.s_ = make_disjoint_interval_set(rhs.s_.begin(), rhs.s_.end()));
        return rhs;
	}
}
