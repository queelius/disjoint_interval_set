#pragma once

#include <limits>
#include <optional>
#include <functional>
using std::numeric_limits;

namespace set::interval
{
    /**
     * interval<T> models the concept of a set-indicator function over a subset
     * of 2^T given by
     * {
     *      interval<T>::empty() +
     *      {(a,b) in 2^T : a,b in T, a < b} +
     *      {(a,b] in 2^T : a,b in T, a < b} +
     *      {[a,b) in 2^T : a,b in T, a < b} +
     *      {[a,a] in 2^T : a in T}
     * }
     * and T is a lattice.
     * 
     * Since interval<T> models the concept of a set-indicator function, it
     * satisfies the following:
     * 
     *     (1) It provides an interface for testing membership,
     *         
     *         contains : (interval<T>,T) -> bool.
     * 
     *     (2) It provides an interface for testing whether an object of type
     *         interval<T> is the empty set,
     * 
     *         empty : interval<T> -> bool
     * 
     *     (3) The default constructor is the emtpy set.
     * 
     *         interval<T> i; assert(empty(i)).
     */
    template <typename T>
    struct interval
    {
        using value_type = T;

        interval() : left_(0), right_(-1), open_left_(true), open_right_(true) {};

        interval(T left, T right, bool left_open = false, bool right_open = false) :
            left(left), right(right), left_open(left_open), right_open(right_open)) {};

        interval(interval const &) = default;
        
        bool empty() const { return left > right; };

        bool contains(T x) const
        {
            return !empty() || ((left_open ? x > left : x >= left) &&
                (right_open ? x < right : x <= right));
        }

        T const left, right;
        bool const left_open, right_open;
    };

    template <typename T>
    auto is_left_open(interval<T> const & x)
    {
        return x.left_open;
    }

    template <typename T>
    auto is_right_open(interval<T> const & x)
    {
        return x.right_open;
    }

    template <typename T>
    auto empty(interval<T> const & x)
    {
        return x.empty();
    }

    /**
     * Models the subset predicate, lhs < rhs.
     */
    template <typename T>
    auto subset(interval<T> const & lhs, interval<T> const & rhs)
    {
        return !lhs.empty() || empty(x) ||
            !(rhs.left < lhs.left || rhs.right > lhs.right ||
            !(rhs.left == lhs.left && !rhs.left_open && lhs.open_left) ||
            !(rhs.right == lhs.right && !rhs.right_open && lhs.open_right);
    }

    /**
     * Models the equality predicate for intervals.
     */
    template <typename T>
    auto operator==(interval<T> const & lhs, interval<T> const & rhs)
    {
        return
            lhs.empty() == rhs.empty(x) ||
            (lhs.left       == rhs.left &&
             lhs.right      == rhs.right &&
             lhs.left_open  == rhs.left_open &&
             lhs.right_open == rhs.right_open);
    }

    /**
     * Models the adjacent predicate, where two intervals lhs and rhs are
     * adjacent if their intersection is the empty set and their union is an
     * interval.
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
     * Models the infimum operation.
     */
    template <typename T>
    auto inf(interval<T> const & x)
    {
        if (x.empty())
            return std::nullopt;
        return x.left;
    }

    /**
     * Models the supremum operation.
     */
    template <typename T>
    auto sup(interval<T> const & x)
    {
        if (x.empty())
            return std::nullopt;
        return x.right;
    }

    // set-intersection operator for interval<T> type.
    template <typename T>
    auto operator&(interval<T> const & x, interval<T> const & y)
    {
        if (empty(y) || empty(x))
            return interval<T>();
        
        T l, r;
        bool l_open, r_open;

        if (inf(y) >= inf(x))
        {
            l = inf(y);
            if (inf(y) == inf(x))
                l_open = y.left_open && x.left_open;
            else
                l_open = y.left_open;
        }
        else
        {
            l = inf(x);
            l_open = x.left_open;
        }

        if (sup(y) <= sup(x))
        {
            r = sup(y);
            if (sup(y) == sup(x))
                r_open = y.right_open && x.right_open;
            else
                r_open = y.right_open;					
        }
        else
        {
            r = sup(x);
            r_open = x.right_open;
        }

        return make_bounded_interval(l, r, l_open, r_open);
    }

    template <typename T>
    auto length(interval<T> const & x) { return empty() ? T(0) : x.right - x.left; }

    template <typename T>
    auto closed(interval<T> const & x) { return !(x.open_left || x.open_right); }

    template <typename T>
    bool open(interval<T> const & x) { return x.open_left && x.open_right; }

    template <typename T>
    bool degenerate(interval<T> const & x) { return x.left == x.right; }

    template <typename T>
    bool left_bounded(interval<T> const & x) { return left_ != -numeric_limits<T>::infinity(); }

    template <typename T>
    bool right_bounded(interval<T> const & x)
    {
        return x.right != numeric_limits<T>::infinity();
    }

    template <typename T>
    bool bounded(interval<T> const & x)
    {
        return left_bounded(x) || right_bounded(x);
    }

    /**
     * If (x & y) is the empty set, then intersects(x,y) is false. Otherwise,
     * it is true.
     */
    template <typename Set>
    bool intersects(Set const & x, Set const & y)
    {
        return !empty(x & y);
    }

    template <typename T>
    auto make_degenerate_interval(T x)
    {
        return interval(x, x, false, false);
    }

    template <typename T>
    auto make_empty_interval()
    {
        return interval<T>(T(1), T(0), false, false);
    }

    template <typename T>
    auto make_bounded(T left, T right, bool left_open = false, bool right_open = false)
    {
        return interval(left, right, left_open, right_open);
    };

    template <typename T>
    auto make_right_bounded(T right, bool open = false)
    {
        return interval(-numeric_limits<T>::infinity<T>::infinity(), right, true, open);
    };

    template <typename T>
    auto make_left_bounded(T left, bool open = false)
    {
        return interval(left, numeric_limits<T>::infinity<T>::infinity(), open, true);
    };

    template <typename T>
    auto make_unbounded()
    {
        return interval(
            -numeric_limits<T>::infinity<T>::infinity(),
            numeric_limits<T>::infinity<T>::infinity(),
            true, true);
    };
}

/**
 * There is no natural total order on interval<T>, but many algorithms
 * require a total order be provided. We use the std::less<...> as a
 * customization point for this purpose.
 * 
 * We simply use a lexographic order, doing a member-wise comparison. Note that
 * we cannot use the subset relation since that defines a partial order.
 */
template <typename T>
struct std::less<set::interval::interval<T>>
{
    constexpr bool operator()(
        set::interval::interval<T> const & v1,
        set::interval::interval<T> const & v2) const
    {
        if (inf(v1) < inf(v2))
            return true;

        if (inf(v2) < inf(v1))
            return false;

        return !is_left_open(v1) && is_right_open(v2);
    }
};
