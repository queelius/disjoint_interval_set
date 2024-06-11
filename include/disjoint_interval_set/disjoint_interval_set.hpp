#pragma once

#include <vector>
#include "disjoint_interval_set_algorithms.hpp"
#include "interval.hpp"

namespace disjoint_interval_set
{
  /**
   * Models the concept of a disjoint set of intervals. It is a Boolean
   * algebra over disjoint interval sets equipped with all the standard
   * set-theoretic operations, like intersection (*), union (+), and
   * complement (~).
   */
  template <typename I = interval<double>>
  class disjoint_interval_set {
    friend auto operator*<>(disjoint_interval_set const &,
                            disjoint_interval_set const &);
  public:
    using interval_type = I;
    using value_type = typename I::value_type;
    using const_iterator = typename std::vector<I>::const_iterator;

    // constuctors
    disjoint_interval_set() = default;
    disjoint_interval_set(const disjoint_interval_set &) = default;

    // accessors
    auto supremum() const { return empty() ? std::nullopt : supremum(s_.back()); }
    auto infimum() const { return empty() ? std::nullopt : infimum(s_.front()); }
    auto contains(value_type v) const {
      return any_of(begin(), end(), [v](I const &i)
                    { return i.contains(v); });
    }
    auto empty() const { return s_.empty(); }
    auto begin() const { return s_.begin(); }
    auto end() const { return s_.end(); }

  private:
    std::vector<I> s_;
  };

  using reals = disjoint_interval_set<interval<double>>;
  using integers = disjoint_interval_set<interval<int>>;

  /**
   * relation predicates
   */

  // subset predicate
  template <typename I, typename A, typename C>
  auto operator<=(disjoint_interval_set<I> const &lhs,
                  disjoint_interval_set<I> const &rhs) {
    auto i = lhs.begin();
    auto j = rhs.begin();
    while (i != lhs.end() && j != rhs.end()) {
      if (i->contains(*j)) ++j;
      else ++i;
    }
    return j == rhs.end();
  }

  // superset predicate
  template <typename I>
  auto operator>=(disjoint_interval_set<I> const &lhs,
                  disjoint_interval_set<I> const &rhs) {
    return rhs <= lhs;
  }

  // equality predicate
  template <typename I>
  auto operator==(disjoint_interval_set<I> const &lhs,
                  disjoint_interval_set<I> const &rhs) {
    return (rhs <= lhs) && (lhs <= rhs);
  }

  // inequality predicate
  template <typename I>
  auto operator!=(disjoint_interval_set<I> const &lhs,
                  disjoint_interval_set<I> const &rhs) {
    return !(lhs == rhs);
  }

  // proper subset predicate
  template <typename I>
  auto operator<(disjoint_interval_set<I> const &lhs,
                 disjoint_interval_set<I> const &rhs) {
    return (lhs <= rhs) && (lhs != rhs);
  }

  // proper superset predicate
  template <typename I>
  auto operator>(disjoint_interval_set<I> const &lhs,
                 disjoint_interval_set<I> const &rhs) {
    return (lhs >= lhs) && (lhs != rhs);
  }

  /**
   * set-theoretic operations
   */

  // intersection
  template <typename I>
  auto operator*(disjoint_interval_set<I> lhs,
                 disjoint_interval_set<I> rhs) {
    return ~((~move(lhs)) + (~move(rhs)));
  }

  template <typename I>
  auto operator^(disjoint_interval_set<I> const &lhs,
                 disjoint_interval_set<I> const &rhs) {
    return (lhs * (~rhs)) + (~(lhs)*rhs);
  }

  // complement
  template <typename T>
  auto operator~(disjoint_interval_set<T> x) {
    x.s_ = complement_disjoint_interval_set(move(x.s_));
    return x;
  }

  // set-difference
  template <typename T>
  auto operator-(disjoint_interval_set<T> lhs, disjoint_interval_set<T> rhs) {
    return move(lhs) * (~move(rhs));
  }

  // union
  template <typename I>
  auto operator+(disjoint_interval_set<I> const &lhs,
                 disjoint_interval_set<I> rhs) {
    if (lhs.empty()) return rhs;
    if (rhs.empty()) return lhs;

    rhs.s_.insert(rhs.s_.end(), lhs.s_.begin(), lhs.s_.end());
    rhs.s_ = disjoint_interval_set<I>(rhs.s_.begin(), rhs.s_.end()));
    return rhs;
  }
}
