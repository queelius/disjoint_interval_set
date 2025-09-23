#include "../include/dis/dis.hpp"
#include <iostream>
#include <cassert>
#include <iomanip>

using namespace dis;

void demonstrate_interval_construction() {
    std::cout << "=== Interval Construction ===\n\n";

    // Multiple ways to create intervals - choose what reads best
    auto closed = real_interval::closed(0, 10);         // [0, 10]
    auto open = real_interval::open(0, 10);            // (0, 10)
    auto left_open = real_interval::left_open(0, 10);  // (0, 10]
    auto right_open = real_interval::right_open(0, 10); // [0, 10)
    auto point = real_interval::point(5);              // {5}
    auto empty = real_interval::empty();               // ∅

    // Special intervals for unbounded ranges
    auto positive = real_interval::greater_than(0);    // (0, ∞)
    auto negative = real_interval::less_than(0);       // (-∞, 0)
    auto non_negative = real_interval::at_least(0);    // [0, ∞)
    auto non_positive = real_interval::at_most(0);     // (-∞, 0]

    std::cout << "Closed interval [0,10]: " << closed << '\n';
    std::cout << "Open interval (0,10): " << open << '\n';
    std::cout << "Point {5}: " << point << '\n';
    std::cout << "Positive numbers: " << positive << '\n';

    // Query operations
    assert(closed.contains(5));
    assert(!open.contains(0));
    assert(closed.length() == 10);
    assert(closed.midpoint() == 5);

    std::cout << "\n";
}

void demonstrate_set_construction() {
    std::cout << "=== Set Construction ===\n\n";

    // Fluent interface for building sets
    auto schedule = real_set{}
        .add(9, 12)      // Morning: 9 AM - 12 PM
        .add(13, 17)     // Afternoon: 1 PM - 5 PM
        .add(19, 21);    // Evening: 7 PM - 9 PM

    std::cout << "Work schedule: " << schedule << '\n';

    // From mathematical notation string
    auto from_string = real_set::from_string("[0,5) U [10,15] U {20}");
    std::cout << "Parsed from string: " << from_string << '\n';

    // From initializer list
    real_set ranges = {
        real_interval::closed(0, 10),
        real_interval::closed(5, 15),  // Overlaps - will be merged
        real_interval::closed(20, 25)
    };
    std::cout << "Merged overlapping: " << ranges << '\n';

    std::cout << "\n";
}

void demonstrate_set_operations() {
    std::cout << "=== Set Operations ===\n\n";

    auto a = real_set::from_string("[0,10] U [20,30]");
    auto b = real_set::from_string("[5,15] U [25,35]");

    // Multiple equivalent notations for clarity
    auto union_ab = a | b;              // Using | operator
    auto union_ab2 = a.unite(b);        // Using named method

    auto intersect_ab = a & b;          // Using & operator
    auto intersect_ab2 = a.intersect(b); // Using named method

    auto diff_ab = a - b;               // Using - operator
    auto diff_ab2 = a.difference(b);    // Using named method

    auto sym_diff = a ^ b;              // Using ^ operator
    auto complement_a = ~a;             // Using ~ operator

    std::cout << "A = " << a << '\n';
    std::cout << "B = " << b << '\n';
    std::cout << "A ∪ B = " << union_ab << '\n';
    std::cout << "A ∩ B = " << intersect_ab << '\n';
    std::cout << "A \\ B = " << diff_ab << '\n';
    std::cout << "A ∆ B = " << sym_diff << '\n';
    std::cout << "~A = " << complement_a << " (unbounded complement)\n";

    std::cout << "\n";
}

void demonstrate_queries() {
    std::cout << "=== Advanced Queries ===\n\n";

    auto set = real_set::from_string("[0,10] U [20,30] U [40,50]");

    // Component analysis
    std::cout << "Set: " << set << '\n';
    std::cout << "Number of components: " << set.component_count() << '\n';
    std::cout << "Span (convex hull): " << set.span() << '\n';
    std::cout << "Gaps between components: " << set.gaps() << '\n';

    // Measure operations
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total measure: " << set.measure() << '\n';
    std::cout << "Gap measure: " << set.gap_measure() << '\n';
    std::cout << "Density (measure/span): " << set.density() << '\n';

    std::cout << "\n";
}

void demonstrate_functional_operations() {
    std::cout << "=== Functional Operations ===\n\n";

    auto set = real_set::from_string("[1,5] U [10,20] U [30,35] U [40,50]");

    // Filter intervals by predicate
    auto large_intervals = set.filter([](const auto& interval) {
        return interval.length() >= 10;
    });
    std::cout << "Intervals with length >= 10: " << large_intervals << '\n';

    // Transform intervals (requires appropriate interval type)
    auto scaled = set.map([](const auto& interval) {
        auto lower = *interval.lower_bound() * 2;
        auto upper = *interval.upper_bound() * 2;
        return real_interval::closed(lower, upper);
    });
    std::cout << "Scaled by 2: " << scaled << '\n';

    // Iterate with action
    std::cout << "Component details:\n";
    set.for_each([](const auto& interval) {
        std::cout << "  " << interval
                  << " - length: " << interval.length()
                  << ", midpoint: " << interval.midpoint() << '\n';
    });

    std::cout << "\n";
}

void demonstrate_real_world_example() {
    std::cout << "=== Real-World Example: Resource Scheduling ===\n\n";

    // Available time slots for a meeting room
    auto room_available = real_set{}
        .add(8, 9)      // 8 AM - 9 AM
        .add(10, 12)    // 10 AM - 12 PM
        .add(14, 17);   // 2 PM - 5 PM

    // Meeting requests
    auto request1 = real_interval::closed(8.5, 9.5);    // 8:30 - 9:30
    auto request2 = real_interval::closed(10, 11);      // 10:00 - 11:00
    auto request3 = real_interval::closed(15, 16.5);    // 3:00 - 4:30

    std::cout << "Room availability: " << room_available << '\n';
    std::cout << "\nMeeting requests:\n";

    // Check if requests can be accommodated
    for (const auto& request : {request1, request2, request3}) {
        bool fits = room_available.contains(request);
        auto available_portion = real_set{request} & room_available;

        std::cout << "  Request " << request << ": ";
        if (fits) {
            std::cout << "✓ Fully available\n";
        } else if (!available_portion.is_empty()) {
            std::cout << "⚠ Partially available: " << available_portion << '\n';
        } else {
            std::cout << "✗ Not available\n";
        }
    }

    // Calculate utilization if all possible meetings are booked
    auto all_requests = real_set{request1} | real_set{request2} | real_set{request3};
    auto booked = all_requests & room_available;

    std::cout << "\nUtilization Analysis:\n";
    std::cout << "  Total available time: " << room_available.measure() << " hours\n";
    std::cout << "  Time that can be booked: " << booked.measure() << " hours\n";
    std::cout << "  Utilization: "
              << (booked.measure() / room_available.measure() * 100) << "%\n";

    std::cout << "\n";
}

void demonstrate_visualization() {
    std::cout << "=== Visualization ===\n\n";

    auto set = real_set::from_string("[10,30] U [40,50] U [70,90]");

    std::cout << "Mathematical notation: " << set << '\n';

    // Different formatting styles
    std::cout << "Unicode style: "
              << interval_formatter<double>::format(set,
                    interval_formatter<double>::Style::Unicode) << '\n';

    std::cout << "Verbose style: "
              << interval_formatter<double>::format(set,
                    interval_formatter<double>::Style::Verbose) << '\n';

    // ASCII visualization
    std::cout << "\nASCII visualization (0-100):\n";
    std::cout << interval_formatter<double>::visualize(set, 0, 100, 60) << '\n';

    std::cout << "\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "   Disjoint Interval Set Library Demo   \n";
    std::cout << "========================================\n\n";

    demonstrate_interval_construction();
    demonstrate_set_construction();
    demonstrate_set_operations();
    demonstrate_queries();
    demonstrate_functional_operations();
    demonstrate_real_world_example();
    demonstrate_visualization();

    std::cout << "========================================\n";
    std::cout << "Demo completed successfully!\n";

    return 0;
}