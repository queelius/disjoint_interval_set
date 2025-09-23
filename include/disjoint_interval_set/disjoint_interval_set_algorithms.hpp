#pragma once

#include <vector>
#include <algorithm>
using std::sort;
using std::numeric_limits;

namespace disjoint_interval_set {

	template <typename Set>
	auto merge_overlapping_intervals(Set s) {
		return make_disjoint_interval_set(s);
	}
	template <typename Set>
	auto make_disjoint_interval_set(Set s) {
		using interval_type = typename Set::value_type;
		sort(s.begin(), s.end(), std::less<interval_type>{});

		auto l = infimum(*s.begin()).value();
		auto r = supremum(*s.begin()).value();
		auto j = s.begin();
		for (const auto& i : s) {
			if (infimum(i).value() > r) {
				*j++ = interval_type(l, r);
				l = infimum(i).value();
				r = supremum(i).value();
			}
			else if (supremum(i).value() > r) {
				r = supremum(i).value();
			}
		}
		*j++ = interval_type(l, r);
		s.erase(j, s.end());
		return s;
	}

	/**
	 * @brief Takes two disjoint set of interval types to produce their union,
     *        another disjoint interval set.
	 * @param s1 A disjoint interval set.
	 * @param s2 A disjoint interval set.
	 * @return The union of s1 and s2, which is another disjoint interval set.
	 */
	template <typename Set1, typename Set2>
	auto union_disjoint_interval_sets(Set1 s1, const Set2& s2) {
		if (s1.empty())	return Set1(s2.begin(), s2.end());
		if (s2.empty()) return s1;
		
		s1.insert(s1.end(), s2.begin(), s2.end());
		return make_disjoint_interval_set(s1);
	};

	template <typename Set>
	using interval_type = typename Set::value_type;

	template <typename Set>
	using interval_value_type = typename interval_type<Set>::value_type;

	/**
	 * @brief Takes the complement of a disjoint set of interval types.
	 * 
	 * @param s A disjoint set of intervals.
	 * @param l The lower limit of the set.
	 * @param u The upper limit of the set.
	 * @return The complement of s, with a lower limit l and an upper limit u.
	 */
	template <typename Set>
	Set complement_disjoint_interval_set(Set s,
		interval_value_type<Set> l = -std::numeric_limits<interval_value_type<Set>>::infinity(),
		interval_value_type<Set> u = std::numeric_limits<interval_value_type<Set>>::infinity()) {
		using interval = interval_type<Set>;

		sort(s.begin(), s.end(),
			[](interval const & v1, interval const & v2) {
					auto inf1 = infimum(v1);
					auto inf2 = infimum(v2);
					if (!inf1.has_value()) return true;
					if (!inf2.has_value()) return false;
					if (inf1.value() < inf2.value()) return true;
					if (inf2.value() < inf1.value()) return false;
					return !is_left_open(v1) && is_left_open(v2);
				});

		Set comp;
		auto lr = l;
		for (const auto& i : s)	{
			if (infimum(i).value() != l) comp.push_back(interval(lr, infimum(i).value()));
			lr = supremum(i).value();
		}
		if (lr != u) comp.push_back(interval(lr, u));
		return comp;
	}
}
