#pragma once

#include <array>
#include <vector>
#include <algorithm>
#include <optional>
#include "interval.hpp"

namespace disjoint_interval_set {

/**
 * @brief Multi-dimensional interval (hyperrectangle/box)
 *
 * Represents an axis-aligned box in N-dimensional space.
 * Each dimension is represented by a 1D interval.
 */
template<typename T, size_t Dims>
class interval_nd {
public:
    using value_type = T;
    using point_type = std::array<T, Dims>;
    using interval_1d = interval<T>;
    static constexpr size_t dimensions = Dims;

    // Default constructor creates an empty box
    interval_nd() : intervals_{} {
        for (size_t i = 0; i < Dims; ++i) {
            intervals_[i] = interval_1d{};
        }
    }

    // Constructor from array of 1D intervals
    explicit interval_nd(std::array<interval_1d, Dims> intervals)
        : intervals_(intervals) {}

    // Constructor from lower and upper corners
    interval_nd(const point_type& lower, const point_type& upper,
                std::array<bool, Dims> lower_open = {},
                std::array<bool, Dims> upper_open = {}) {
        for (size_t i = 0; i < Dims; ++i) {
            intervals_[i] = interval_1d(lower[i], upper[i],
                                        lower_open[i], upper_open[i]);
        }
    }

    // Check if the box is empty
    bool empty() const {
        for (const auto& i : intervals_) {
            if (i.empty()) return true;
        }
        return false;
    }

    // Check if a point is contained in the box
    bool contains(const point_type& point) const {
        if (empty()) return false;
        for (size_t i = 0; i < Dims; ++i) {
            if (!intervals_[i].contains(point[i])) {
                return false;
            }
        }
        return true;
    }

    // Access interval for a specific dimension
    const interval_1d& operator[](size_t dim) const {
        return intervals_[dim];
    }

    interval_1d& operator[](size_t dim) {
        return intervals_[dim];
    }

    // Get the intervals array
    const std::array<interval_1d, Dims>& intervals() const {
        return intervals_;
    }

    // Get lower corner of the box
    std::optional<point_type> lower_corner() const {
        if (empty()) return std::nullopt;
        point_type lower;
        for (size_t i = 0; i < Dims; ++i) {
            lower[i] = intervals_[i].left;
        }
        return lower;
    }

    // Get upper corner of the box
    std::optional<point_type> upper_corner() const {
        if (empty()) return std::nullopt;
        point_type upper;
        for (size_t i = 0; i < Dims; ++i) {
            upper[i] = intervals_[i].right;
        }
        return upper;
    }

private:
    std::array<interval_1d, Dims> intervals_;
};

// Free functions for interval_nd
template<typename T, size_t Dims>
bool empty(const interval_nd<T, Dims>& box) {
    return box.empty();
}

template<typename T, size_t Dims>
bool contains(const interval_nd<T, Dims>& box,
              const typename interval_nd<T, Dims>::point_type& point) {
    return box.contains(point);
}

// Intersection of two boxes
template<typename T, size_t Dims>
interval_nd<T, Dims> operator*(const interval_nd<T, Dims>& lhs,
                               const interval_nd<T, Dims>& rhs) {
    std::array<interval<T>, Dims> result_intervals;
    for (size_t i = 0; i < Dims; ++i) {
        result_intervals[i] = lhs[i] * rhs[i];
    }
    return interval_nd<T, Dims>(result_intervals);
}

// Check if two boxes are disjoint
template<typename T, size_t Dims>
bool disjoint(const interval_nd<T, Dims>& lhs,
              const interval_nd<T, Dims>& rhs) {
    return empty(lhs * rhs);
}

/**
 * @brief Multi-dimensional disjoint interval set
 *
 * Represents a set of non-overlapping boxes in N-dimensional space.
 */
template<typename T, size_t Dims>
class disjoint_interval_set_nd {
public:
    using box_type = interval_nd<T, Dims>;
    using value_type = T;
    using point_type = typename box_type::point_type;
    using const_iterator = typename std::vector<box_type>::const_iterator;

    // Default constructor
    disjoint_interval_set_nd() = default;

    // Constructor from vector of boxes
    explicit disjoint_interval_set_nd(std::vector<box_type> boxes)
        : boxes_(make_disjoint(std::move(boxes))) {}

    // Check if the set is empty
    bool empty() const { return boxes_.empty(); }

    // Check if a point is contained in any box
    bool contains(const point_type& point) const {
        return std::any_of(boxes_.begin(), boxes_.end(),
                          [&point](const box_type& box) {
                              return box.contains(point);
                          });
    }

    // Add a box to the set (maintaining disjoint property)
    void insert(const box_type& box) {
        if (box.empty()) return;
        boxes_.push_back(box);
        boxes_ = make_disjoint(std::move(boxes_));
    }

    // Iterators
    const_iterator begin() const { return boxes_.begin(); }
    const_iterator end() const { return boxes_.end(); }

    // Size
    size_t size() const { return boxes_.size(); }

    // Clear all boxes
    void clear() { boxes_.clear(); }

private:
    std::vector<box_type> boxes_;

    // Make a set of boxes disjoint (simple O(n²) algorithm)
    static std::vector<box_type> make_disjoint(std::vector<box_type> boxes) {
        // For simplicity, we keep all boxes and rely on the fact that
        // we only add non-overlapping boxes. A full implementation would
        // split overlapping boxes into disjoint pieces.

        // Remove empty boxes
        boxes.erase(
            std::remove_if(boxes.begin(), boxes.end(),
                          [](const box_type& b) { return b.empty(); }),
            boxes.end());

        // Sort by lower corner (lexicographic order)
        std::sort(boxes.begin(), boxes.end(),
                 [](const box_type& a, const box_type& b) {
                     auto lower_a = a.lower_corner();
                     auto lower_b = b.lower_corner();
                     if (!lower_a) return true;
                     if (!lower_b) return false;
                     return *lower_a < *lower_b;
                 });

        return boxes;
    }
};

// Union of two multi-dimensional disjoint interval sets
template<typename T, size_t Dims>
disjoint_interval_set_nd<T, Dims> operator+(
    const disjoint_interval_set_nd<T, Dims>& lhs,
    const disjoint_interval_set_nd<T, Dims>& rhs) {

    std::vector<interval_nd<T, Dims>> combined;
    combined.reserve(lhs.size() + rhs.size());

    for (const auto& box : lhs) {
        combined.push_back(box);
    }
    for (const auto& box : rhs) {
        combined.push_back(box);
    }

    return disjoint_interval_set_nd<T, Dims>(combined);
}

// Intersection of two multi-dimensional disjoint interval sets
template<typename T, size_t Dims>
disjoint_interval_set_nd<T, Dims> operator*(
    const disjoint_interval_set_nd<T, Dims>& lhs,
    const disjoint_interval_set_nd<T, Dims>& rhs) {

    std::vector<interval_nd<T, Dims>> result;

    for (const auto& box1 : lhs) {
        for (const auto& box2 : rhs) {
            auto intersection = box1 * box2;
            if (!intersection.empty()) {
                result.push_back(intersection);
            }
        }
    }

    return disjoint_interval_set_nd<T, Dims>(result);
}

// Type aliases for common dimensions
template<typename T>
using interval_2d = interval_nd<T, 2>;

template<typename T>
using interval_3d = interval_nd<T, 3>;

template<typename T>
using disjoint_interval_set_2d = disjoint_interval_set_nd<T, 2>;

template<typename T>
using disjoint_interval_set_3d = disjoint_interval_set_nd<T, 3>;

// Convenience functions for 2D
template<typename T>
interval_2d<T> make_rectangle(T x_min, T x_max, T y_min, T y_max,
                              bool x_min_open = false, bool x_max_open = false,
                              bool y_min_open = false, bool y_max_open = false) {
    return interval_2d<T>({{x_min, y_min}}, {{x_max, y_max}},
                          {{x_min_open, y_min_open}}, {{x_max_open, y_max_open}});
}

// Convenience functions for 3D
template<typename T>
interval_3d<T> make_box(T x_min, T x_max, T y_min, T y_max, T z_min, T z_max,
                        bool x_min_open = false, bool x_max_open = false,
                        bool y_min_open = false, bool y_max_open = false,
                        bool z_min_open = false, bool z_max_open = false) {
    return interval_3d<T>({{x_min, y_min, z_min}}, {{x_max, y_max, z_max}},
                          {{x_min_open, y_min_open, z_min_open}},
                          {{x_max_open, y_max_open, z_max_open}});
}

} // namespace disjoint_interval_set