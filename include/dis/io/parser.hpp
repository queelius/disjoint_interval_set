#pragma once

#include "../core/interval.hpp"
#include "../core/disjoint_interval_set.hpp"
#include <string>
#include <string_view>
#include <charconv>
#include <stdexcept>
#include <sstream>
#include <regex>

namespace dis {

/**
 * @brief Parser for mathematical interval notation.
 *
 * Supports standard mathematical notation:
 * - [a,b] : closed interval
 * - (a,b) : open interval
 * - [a,b) : left-closed, right-open
 * - (a,b] : left-open, right-closed
 * - {a} : singleton/point
 * - ∅ or {} : empty set
 *
 * Set operations:
 * - ∪ or U : union
 * - ∩ or ∩ : intersection
 * - \ : difference
 * - ∆ : symmetric difference
 *
 * Examples:
 * - "[0,10) ∪ [20,30]"
 * - "(1,5] ∩ [3,7)"
 * - "[0,100] \ [40,60]"
 */
template<typename T>
class interval_parser {
public:
    using interval_type = interval<T>;
    using set_type = disjoint_interval_set<interval_type>;

    // Parse a single interval from string
    [[nodiscard]] static interval_type parse_interval(std::string_view str) {
        str = trim(str);

        if (str.empty() || str == "{}" || str == "∅") {
            return interval_type::empty();
        }

        // Handle singleton {value}
        if (str.starts_with('{') && str.ends_with('}')) {
            auto value_str = str.substr(1, str.size() - 2);
            T value = parse_value(trim(value_str));
            return interval_type::point(value);
        }

        // Handle interval notation [a,b], (a,b), etc.
        if ((str.starts_with('[') || str.starts_with('(')) &&
            (str.ends_with(']') || str.ends_with(')'))) {

            bool left_closed = str[0] == '[';
            bool right_closed = str[str.size() - 1] == ']';

            auto inner = str.substr(1, str.size() - 2);
            auto comma_pos = inner.find(',');

            if (comma_pos == std::string_view::npos) {
                throw std::invalid_argument("Invalid interval format: missing comma");
            }

            auto lower_str = trim(inner.substr(0, comma_pos));
            auto upper_str = trim(inner.substr(comma_pos + 1));

            T lower = parse_value(lower_str);
            T upper = parse_value(upper_str);

            return interval_type(lower, upper, left_closed, right_closed);
        }

        throw std::invalid_argument("Invalid interval format");
    }

    // Parse a disjoint interval set from string
    [[nodiscard]] static set_type parse_set(std::string_view str) {
        str = trim(str);

        if (str.empty() || str == "{}" || str == "∅") {
            return set_type::empty();
        }

        // Handle complex expressions with operations
        if (contains_set_operation(str)) {
            if constexpr (std::numeric_limits<T>::has_infinity) {
                return parse_expression(str);
            } else {
                throw std::invalid_argument("Set operations require unbounded type");
            }
        }

        // Simple case: single interval or union of intervals
        set_type result;
        size_t pos = 0;

        while (pos < str.size()) {
            auto next_union = find_next_union(str, pos);
            auto interval_str = str.substr(pos, next_union - pos);

            result = result.unite(set_type{parse_interval(interval_str)});

            if (next_union == std::string_view::npos) {
                break;
            }
            pos = next_union + 1;  // Skip union symbol
        }

        return result;
    }

private:
    static T parse_value(std::string_view str) {
        str = trim(str);

        // Handle special values
        if constexpr (std::numeric_limits<T>::has_infinity) {
            if (str == "∞" || str == "+∞" || str == "inf") {
                return std::numeric_limits<T>::infinity();
            }
            if (str == "-∞" || str == "-inf") {
                return -std::numeric_limits<T>::infinity();
            }
        }

        // Parse numeric value
        T value;
        if constexpr (std::is_integral_v<T>) {
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
            if (ec != std::errc{}) {
                throw std::invalid_argument("Failed to parse integer value");
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            // For floating point, use stringstream as fallback since from_chars
            // support for float is not universal
            std::stringstream ss;
            ss << str;
            if (!(ss >> value)) {
                throw std::invalid_argument("Failed to parse floating point value");
            }
        } else {
            // For custom types, assume stream extraction operator
            std::stringstream ss;
            ss << str;
            if (!(ss >> value)) {
                throw std::invalid_argument("Failed to parse value");
            }
        }

        return value;
    }

    static std::string_view trim(std::string_view str) {
        auto start = str.find_first_not_of(" \t\n\r");
        if (start == std::string_view::npos) return "";

        auto end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }

    static bool contains_set_operation(std::string_view str) {
        // Check for intersection, difference, symmetric difference
        // (Union is handled separately as it's the default)
        return str.find("∩") != std::string_view::npos ||
               str.find('\\') != std::string_view::npos ||
               str.find("∆") != std::string_view::npos ||
               str.find('&') != std::string_view::npos ||
               str.find('^') != std::string_view::npos;
    }

    static size_t find_next_union(std::string_view str, size_t start) {
        // Find next union symbol not within brackets/parentheses
        int depth = 0;
        for (size_t i = start; i < str.size(); ++i) {
            char c = str[i];
            if (c == '[' || c == '(' || c == '{') {
                depth++;
            } else if (c == ']' || c == ')' || c == '}') {
                depth--;
            } else if (depth == 0) {
                if (c == 'U' || c == '|') {
                    return i;
                }
                // Check for Unicode union symbol
                if (i + 2 < str.size() && str.substr(i, 3) == "∪") {
                    return i;
                }
            }
        }
        return std::string_view::npos;
    }

    static set_type parse_expression(std::string_view str)
        requires std::numeric_limits<T>::has_infinity {
        // This is a simplified expression parser
        // A full implementation would use proper recursive descent or Pratt parsing

        // For now, handle simple binary operations
        size_t op_pos = std::string_view::npos;
        enum Op { NONE, INTERSECT, DIFFERENCE, SYM_DIFF } op_type = NONE;
        int depth = 0;

        // Find the main operation (rightmost at depth 0)
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            if (c == '[' || c == '(' || c == '{') {
                depth++;
            } else if (c == ']' || c == ')' || c == '}') {
                depth--;
            } else if (depth == 0) {
                if (c == '&') {
                    op_pos = i;
                    op_type = INTERSECT;
                } else if (c == '\\' || c == '-') {
                    op_pos = i;
                    op_type = DIFFERENCE;
                } else if (c == '^') {
                    op_pos = i;
                    op_type = SYM_DIFF;
                } else if (i + 2 < str.size()) {
                    if (str.substr(i, 3) == "∩") {
                        op_pos = i;
                        op_type = INTERSECT;
                    } else if (str.substr(i, 3) == "∆") {
                        op_pos = i;
                        op_type = SYM_DIFF;
                    }
                }
            }
        }

        if (op_pos != std::string_view::npos) {
            size_t op_len = (op_type == INTERSECT && str.substr(op_pos, 3) == "∩") ? 3 :
                           (op_type == SYM_DIFF && str.substr(op_pos, 3) == "∆") ? 3 : 1;

            auto left_str = trim(str.substr(0, op_pos));
            auto right_str = trim(str.substr(op_pos + op_len));

            auto left = parse_set(left_str);
            auto right = parse_set(right_str);

            switch (op_type) {
                case INTERSECT: return left.intersect(right);
                case DIFFERENCE: return left.difference(right);
                case SYM_DIFF: return left.symmetric_difference(right);
                default: return left;
            }
        }

        // No operation found, parse as simple set
        return parse_set(str);
    }
};

// Template deduction guide
template<typename T>
interval_parser(interval<T>) -> interval_parser<T>;

// Convenience functions for string parsing
template<typename T>
[[nodiscard]] inline interval<T> parse_interval(std::string_view str) {
    return interval_parser<T>::parse_interval(str);
}

template<typename T>
[[nodiscard]] inline disjoint_interval_set<interval<T>> parse_set(std::string_view str) {
    return interval_parser<T>::parse_set(str);
}

// Define the from_string method referenced in the core header
template<typename I>
[[nodiscard]] disjoint_interval_set<I> disjoint_interval_set<I>::from_string(std::string_view str) {
    return interval_parser<value_type>::parse_set(str);
}

} // namespace dis