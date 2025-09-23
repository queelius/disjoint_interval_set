#pragma once

#include "../core/interval.hpp"
#include "../core/disjoint_interval_set.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

namespace dis {

/**
 * @brief Formatting utilities for intervals and interval sets.
 *
 * Provides multiple output formats:
 * - Mathematical notation: [a,b], (a,b), etc.
 * - Unicode notation: with proper mathematical symbols
 * - ASCII art visualization for numeric types
 * - LaTeX output for documentation
 */
template<typename T>
class interval_formatter {
public:
    enum class Style {
        Mathematical,  // Standard [a,b] notation
        Unicode,       // With ∪, ∩, ∅ symbols
        Programming,   // C-style notation
        LaTeX,         // For document generation
        Verbose        // Human-readable description
    };

    // Format a single interval
    [[nodiscard]] static std::string format(const interval<T>& i, Style style = Style::Mathematical) {
        if (i.is_empty()) {
            switch (style) {
                case Style::Unicode: return "∅";
                case Style::LaTeX: return "\\emptyset";
                case Style::Verbose: return "empty interval";
                default: return "{}";
            }
        }

        if (i.is_point()) {
            auto value = *i.lower_bound();
            switch (style) {
                case Style::LaTeX: return "\\{" + value_to_string(value) + "\\}";
                case Style::Verbose: return "point at " + value_to_string(value);
                default: return "{" + value_to_string(value) + "}";
            }
        }

        std::ostringstream oss;
        auto lower = *i.lower_bound();
        auto upper = *i.upper_bound();

        switch (style) {
            case Style::Mathematical:
            case Style::Unicode:
                oss << (i.is_left_closed() ? '[' : '(')
                    << value_to_string(lower)
                    << ','
                    << value_to_string(upper)
                    << (i.is_right_closed() ? ']' : ')');
                break;

            case Style::Programming:
                oss << "interval(" << value_to_string(lower)
                    << ", " << value_to_string(upper)
                    << ", " << (i.is_left_closed() ? "true" : "false")
                    << ", " << (i.is_right_closed() ? "true" : "false") << ")";
                break;

            case Style::LaTeX:
                oss << (i.is_left_closed() ? "[" : "(")
                    << value_to_string(lower)
                    << ", "
                    << value_to_string(upper)
                    << (i.is_right_closed() ? "]" : ")");
                break;

            case Style::Verbose:
                oss << "interval from " << value_to_string(lower)
                    << " (" << (i.is_left_closed() ? "inclusive" : "exclusive") << ")"
                    << " to " << value_to_string(upper)
                    << " (" << (i.is_right_closed() ? "inclusive" : "exclusive") << ")";
                break;
        }

        return oss.str();
    }

    // Format a disjoint interval set
    [[nodiscard]] static std::string format(const disjoint_interval_set<interval<T>>& set,
                                           Style style = Style::Mathematical) {
        if (set.is_empty()) {
            switch (style) {
                case Style::Unicode: return "∅";
                case Style::LaTeX: return "\\emptyset";
                case Style::Verbose: return "empty set";
                default: return "{}";
            }
        }

        std::ostringstream oss;
        bool first = true;

        for (const auto& interval : set) {
            if (!first) {
                switch (style) {
                    case Style::Unicode: oss << " ∪ "; break;
                    case Style::LaTeX: oss << " \\cup "; break;
                    case Style::Verbose: oss << " union "; break;
                    default: oss << " U "; break;
                }
            }
            oss << format(interval, style);
            first = false;
        }

        return oss.str();
    }

    // Generate ASCII visualization for numeric types
    [[nodiscard]] static std::string visualize(const disjoint_interval_set<interval<T>>& set,
                                               T min_val, T max_val, int width = 80)
        requires std::is_arithmetic_v<T> {
        if (width < 20) width = 20;

        std::ostringstream oss;
        std::vector<char> line(width, '.');

        // Mark the intervals
        for (const auto& interval : set) {
            auto lower = *interval.lower_bound();
            auto upper = *interval.upper_bound();

            int start = static_cast<int>((lower - min_val) / (max_val - min_val) * (width - 1));
            int end = static_cast<int>((upper - min_val) / (max_val - min_val) * (width - 1));

            start = std::max(0, std::min(width - 1, start));
            end = std::max(0, std::min(width - 1, end));

            line[start] = interval.is_left_closed() ? '[' : '(';
            line[end] = interval.is_right_closed() ? ']' : ')';

            for (int i = start + 1; i < end; ++i) {
                line[i] = '=';
            }
        }

        // Build the output
        oss << std::string(line.begin(), line.end()) << '\n';

        // Add scale
        oss << std::setw(0) << min_val;
        int middle_pos = width / 2;
        T middle_val = min_val + (max_val - min_val) / 2;
        oss << std::string(middle_pos - std::to_string(min_val).length() -
                          std::to_string(middle_val).length() / 2, ' ')
            << middle_val;
        oss << std::string(width - middle_pos - std::to_string(middle_val).length() / 2 -
                          std::to_string(max_val).length(), ' ')
            << max_val;

        return oss.str();
    }

private:
    static std::string value_to_string(T value) {
        if constexpr (std::numeric_limits<T>::has_infinity) {
            if (value == std::numeric_limits<T>::infinity()) {
                return "∞";
            }
            if (value == -std::numeric_limits<T>::infinity()) {
                return "-∞";
            }
        }

        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
};

// Stream insertion operators
template<typename T>
std::ostream& operator<<(std::ostream& os, const interval<T>& i) {
    os << interval_formatter<T>::format(i);
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const disjoint_interval_set<interval<T>>& set) {
    os << interval_formatter<T>::format(set);
    return os;
}

} // namespace dis