#pragma once

#include <limits>
#include <optional>
#include <functional>
using std::numeric_limits;

namespace disjoint_interval_set
{
  /**
   * @brief The interval<T> class models the mathematical concept of an interval 
   *        over a type T, acting as a set-indicator function over a subset of 2^T.
   * 
   * The set of all intervals over T is defined as the union of:
   *  - interval<T>::empty()
   *  - {(a,b) in 2^T : a,b in T, a < b}
   *  - {(a,b] in 2^T : a,b in T, a < b}
   *  - {[a,b) in 2^T : a,b in T, a < b}
   *  - {[a,a] in 2^T : a in T}
   * 
   * Here, T is assumed to form a lattice, making it suitable for interval operations.
   * 
   * The interval<T> class satisfies the following properties, inherent to a
   * set-indicator function:
   * 
   * - Membership Testing: 
   *   A method to test whether a given element of type T is contained
   *   within the interval.
   *   @code
   *   auto contained = contains(interval_obj, element);
   *   @endcode
   *
   * - Emptiness Testing:
   *   A method to check if the interval represents an empty set.
   *   @code
   *   auto is_empty = empty(interval_obj);
   *   @endcode
   *         
   * - Default Construction:
   *   The default constructor creates an empty interval.
   *   @code
   *   interval<T> i; 
   *   assert(empty(i));
   *   @endcode
   * 
   * - Infimum and Supremum:
   *   Provides the infimum (lowest) and supremum (highest) values of an interval.
   *   @code
   *   auto inf_val = infimum(i);
   *   auto sup_val = supremum(i);
   *   @endcode
   *
   * Additionally, the interval<T> class provides several other operations:
   * 
   * - Interval Construction:
   *   Allows the creation of an interval with specified left and right endpoints, 
   *   with options to include or exclude these endpoints.
   *   @code
   *   interval<T> i(left, right, left_open, right_open);
   *   @endcode
   *
   * - Copy Construction:
   *   Allows the creation of a new interval that is a copy of an existing interval.
   *   @code
   *   interval<T> j(i);  // where i is an existing interval
   *   @endcode
   *
   * - Interval Comparison:
   *   Provides operations to compare intervals, either by subset relation or equality.
   *   @code
   *   bool is_subset = (i < j);
   *   bool is_equal = (i == j);
   *   @endcode
   *  
   * - Adjacency Testing:
   *   Checks if two intervals are adjacent.
   *   @code
   *   bool is_adjacent = adjacent(i, j);
   *   @endcode
   *
   * - Intersection:
   *   Computes the intersection of two intervals.
   *   @code
   *   auto intersection = (i * j);
   *   @endcode
   *     
   * - Lexicographical Comparison:
   *   Provides a lexicographical comparison of intervals for algorithms
   *   that require a total order.
   *   @code
   *   std::less<interval<T>> lex_compare;
   *   bool lex_less = lex_compare(i, j);
   *   @endcode
   *
   * These operations serve as a computational basis, facilitating a variety of
   * manipulations and queries on intervals, and are particularly useful in the
   * context of the disjoint_interval_set class.
   */
  template <typename T>
  struct interval
  {
      using value_type = T;

      /**
       * The default constructor is the empty set.
       * 
       * @return interval<T>
       */
      interval() : left(0), right(-1), left_open(true), right_open(true) {};

      /**
       * Constructs an interval containing all elements between left and right,
       * containing the endpoints if left_open and right_open are false.
       * 
       * @param left The left endpoint of the interval.
       * @param right The right endpoint of the interval.
       * @param left_open If true, the left endpoint is not contained in the
       *                  interval. Defaults to false.
       * @param right_open If true, the right endpoint is not contained in the
       *                   interval. Defaults to false.
       * @return interval<T>
       */
      interval(T left, T right, bool left_open = false, bool right_open = false) :
          left(left), right(right), left_open(left_open), right_open(right_open) {};

      /**
       * @brief Copy constructor.
       * 
       * @param copy The interval to copy.
       * @return interval<T>
       */
      interval(interval const & copy) = default;
      
      /**
       * @brief Checks if the interval is empty.
       * 
       * @return true if the interval is empty, false otherwise.
       */
      bool empty() const { return left > right; };

      /**
       * @brief Checks if a value is contained within the interval.
       * 
       * @param x The value to check.
       * @return true if x is contained within the interval, false otherwise.
       */
      bool contains(T x) const
      {
        return !empty() && (left_open ? x > left : x >= left) &&
          (right_open ? x < right : x <= right);
      }

      /**
       * @brief The left endpoint of the interval.
       */
      T left, right;

      /**
       * @brief The left endpoint is open.
       */
      bool left_open, right_open;
  };

  /**
   * @brief Check if the left endpoint is open.
   *
   * @param x The interval to check.
   * @return true if the left endpoint is open, false otherwise.
   */
  template <typename T>
  auto is_left_open(interval<T> const & x) { return x.left_open; }

  /**
   * @brief Check if the right endpoint is open.
   *
   * @param x The interval to check.
   * @return true if the right endpoint is open, false otherwise.
   */
  template <typename T>
  auto is_right_open(interval<T> const & x) { return x.right_open; }

  /**
   * @brief Checks if a value is contained within a given interval.
   *
   * @param x The interval to check.
   * @param y The value to check.
   * @return true if y is contained within interval x, false otherwise.
   */
  template <typename T>
  auto contains(interval<T> const & x, T y) { return x.contains(y); }

  /**
   * @brief Checks if an interval is empty.
   *
   * @param x The interval to check.
   * @return true if the interval x is empty, false otherwise.
   */
  template <typename T>
  auto empty(interval<T> const & x) { return x.empty(); }

/**
   * @brief Computes the infimum of an interval.
   *
   * @param x The interval.
   * @return The left endpoint of interval x, or std::nullopt if x is empty.
   */
  template <typename T>
  auto infimum(interval<T> const & x) { return x.empty() ? std::optional<T>{} : std::optional<T>{x.left}; }

/**
   * @brief Computes the supremum of an interval.
   *
   * @param x The interval.
   * @return The right endpoint of interval x, or std::nullopt if x is empty.
   */
  template <typename T>
  auto supremum(interval<T> const & x) { return x.empty() ? std::optional<T>{} : std::optional<T>{x.right}; }

/**
   * @brief Checks if one interval is a subset of another.
   *
   * @param lhs The first interval.
   * @param rhs The second interval.
   * @return true if interval lhs is a subset of interval rhs, false otherwise.
   */
  template <typename T>
  auto operator<(interval<T> const & lhs, interval<T> const & rhs)
  {
    if (lhs.empty()) return true;
    if (rhs.empty()) return false;
    bool left_ok = (rhs.left < lhs.left) || (rhs.left == lhs.left && (rhs.left_open || !lhs.left_open));
    bool right_ok = (rhs.right > lhs.right) || (rhs.right == lhs.right && (rhs.right_open || !lhs.right_open));
    return left_ok && right_ok;
  }

  /**
   * @brief Checks if two intervals are equal.
   *
   * @param lhs The first interval.
   * @param rhs The second interval.
   * @return true if the intervals are equal, false otherwise.
   */
  template <typename T>
  auto operator==(interval<T> const & lhs, interval<T> const & rhs)
  {
    return (lhs.empty() && rhs.empty()) ||
      (lhs.left == rhs.left && lhs.right == rhs.right &&
       lhs.left_open == rhs.left_open && lhs.right_open == rhs.right_open);
  }

  /**
   * @brief Checks if two intervals are adjacent.
   *
   * @param lhs The first interval.
   * @param rhs The second interval.
   * @return true if the intervals are adjacent, false otherwise.
   */
  template <typename T>
  auto adjacent(interval<T> const & lhs, interval<T> const & rhs)
  {
    if (lhs.right == rhs.left)
      return lhs.right_open != rhs.left_open;
    if (lhs.left == rhs.right)
      return lhs.left_open != rhs.right_open;
    return false;
  }

  /**
   * @brief Computes the intersection of two intervals.
   *
   * @param x The first interval.
   * @param y The second interval.
   * @return The intersection of intervals x and y, or an empty interval if they do not intersect.
   */
  template <typename T>
  auto operator*(interval<T> const & x, interval<T> const & y)
  {
    if (empty(y) || empty(x))
      return interval<T>();
    
    T l, r;
    bool l_open, r_open;

    if (infimum(y).value_or(0) >= infimum(x).value_or(0)) {
      l = infimum(y).value();
      if (infimum(y) == infimum(x))
        l_open = y.left_open && x.left_open;
      else
        l_open = y.left_open;
    }
    else {
      l = infimum(x).value();
      l_open = x.left_open;
    }

    if (supremum(y).value_or(0) <= supremum(x).value_or(0)) {
      r = supremum(y).value();
      if (supremum(y) == supremum(x))
        r_open = y.right_open && x.right_open;
      else
        r_open = y.right_open;					
    }
    else {
      r = supremum(x).value();
      r_open = x.right_open;
    }

    return interval<T>(l, r, l_open, r_open);
  }
};


/**
 * @brief Compares two intervals using a lexicographical order.
 *
 * @param v1 The first interval.
 * @param v2 The second interval.
 * @return true if interval v1 comes before interval v2 in the lexicographical order, false otherwise.
 */
template <typename T>
struct std::less<disjoint_interval_set::interval<T>>
{
  constexpr bool operator()(
    disjoint_interval_set::interval<T> const & v1,
    disjoint_interval_set::interval<T> const & v2) const
  {
    auto inf1 = infimum(v1);
    auto inf2 = infimum(v2);
    if (!inf1.has_value()) return true;
    if (!inf2.has_value()) return false;
    if (inf1.value() < inf2.value()) return true;
    else if (inf2.value() < inf1.value()) return false;
    else return !v1.left_open && v2.left_open;
  }
};
