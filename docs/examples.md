# Examples

Complete working examples demonstrating real-world usage of the DIS library.

## Basic Usage

### Example 1: Time Slot Management

```cpp
#include <dis/dis.hpp>
#include <iostream>

using namespace dis;

int main() {
    // Define available time slots
    auto availability = real_set{}
        .add(9, 12)   // Morning: 9 AM - 12 PM
        .add(14, 17); // Afternoon: 2 PM - 5 PM

    // Parse meeting requests
    auto meetings = real_set::from_string("[10,11] ∪ [15,16]");

    // Calculate free time
    auto free_time = availability - meetings;

    // Display results
    std::cout << "Availability: " << availability << '\n';
    std::cout << "Meetings: " << meetings << '\n';
    std::cout << "Free time: " << free_time << '\n';
    std::cout << "Total free hours: " << free_time.measure() << '\n';

    // Check specific times
    if (availability.contains(10.5)) {
        std::cout << "10:30 AM is available\n";
    }

    // Analyze gaps
    auto gaps = meetings.gaps();
    std::cout << "Gaps between meetings: " << gaps << '\n';

    // Calculate utilization
    double utilization = meetings.measure() / availability.measure();
    std::cout << "Room utilization: " << (utilization * 100) << "%\n";

    return 0;
}
```

**Output:**
```
Availability: [9,12] ∪ [14,17]
Meetings: [10,11] ∪ [15,16]
Free time: [9,10) ∪ (11,12] ∪ [14,15) ∪ (16,17]
Total free hours: 4
10:30 AM is available
Gaps between meetings: (11,15)
Room utilization: 33.33%
```

---

## Scheduling Application

### Example 2: Conference Room Scheduler

```cpp
#include <dis/dis.hpp>
#include <map>
#include <string>
#include <vector>
#include <iostream>

using namespace dis;

class ConferenceRoomScheduler {
    struct Room {
        std::string name;
        real_set schedule;
        int capacity;
    };

    std::vector<Room> rooms_;
    real_set business_hours_;

public:
    ConferenceRoomScheduler()
        : business_hours_(real_set{}
            .add(8, 12)
            .add(13, 18))  // 8 AM - 12 PM, 1 PM - 6 PM
    {}

    void add_room(const std::string& name, int capacity) {
        rooms_.push_back({name, real_set{}, capacity});
    }

    bool book_meeting(const std::string& room_name,
                     double start, double end,
                     int required_capacity) {
        auto meeting = real_interval::closed(start, end);

        // Check business hours
        if (!business_hours_.contains(start) ||
            !business_hours_.contains(end - 0.01)) {
            std::cout << "Outside business hours\n";
            return false;
        }

        // Find room
        for (auto& room : rooms_) {
            if (room.name == room_name) {
                if (room.capacity < required_capacity) {
                    std::cout << "Insufficient capacity\n";
                    return false;
                }

                // Check availability
                if (!room.schedule.overlaps(real_set{meeting})) {
                    room.schedule.insert(meeting);
                    std::cout << "Booked: " << room_name
                             << " from " << start << " to " << end << '\n';
                    return true;
                } else {
                    std::cout << "Room not available\n";
                    return false;
                }
            }
        }

        std::cout << "Room not found\n";
        return false;
    }

    std::vector<std::string> find_available_rooms(double start, double end,
                                                   int required_capacity) {
        auto meeting = real_interval::closed(start, end);
        std::vector<std::string> available;

        for (const auto& room : rooms_) {
            if (room.capacity >= required_capacity &&
                !room.schedule.overlaps(real_set{meeting})) {
                available.push_back(room.name);
            }
        }

        return available;
    }

    void print_schedule() {
        for (const auto& room : rooms_) {
            std::cout << room.name << " (capacity: " << room.capacity << "): "
                     << room.schedule << '\n';
        }
    }
};

int main() {
    ConferenceRoomScheduler scheduler;

    // Add rooms
    scheduler.add_room("Room A", 10);
    scheduler.add_room("Room B", 20);
    scheduler.add_room("Room C", 5);

    // Book meetings
    scheduler.book_meeting("Room A", 9, 10, 8);   // Success
    scheduler.book_meeting("Room A", 9.5, 10.5, 8); // Conflict
    scheduler.book_meeting("Room B", 10, 11, 15); // Success
    scheduler.book_meeting("Room C", 14, 15, 3);  // Success

    std::cout << "\nFinal Schedule:\n";
    scheduler.print_schedule();

    std::cout << "\nAvailable rooms for 11-12, capacity 10:\n";
    auto available = scheduler.find_available_rooms(11, 12, 10);
    for (const auto& room : available) {
        std::cout << "  " << room << '\n';
    }

    return 0;
}
```

---

## Computational Geometry

### Example 3: 1D Line Segment Operations

```cpp
#include <dis/dis.hpp>
#include <iostream>

using namespace dis;

void demonstrate_geometric_operations() {
    // Define line segments
    auto segment1 = real_set{real_interval::closed(0, 10)};
    auto segment2 = real_set{real_interval::closed(5, 15)};
    auto segment3 = real_set{real_interval::closed(20, 30)};

    std::cout << "Segment 1: " << segment1 << '\n';
    std::cout << "Segment 2: " << segment2 << '\n';
    std::cout << "Segment 3: " << segment3 << '\n';
    std::cout << '\n';

    // Union (coverage)
    auto coverage = segment1 | segment2 | segment3;
    std::cout << "Total coverage: " << coverage << '\n';

    // Intersection (overlap)
    auto overlap = segment1 & segment2;
    std::cout << "Overlap (1 & 2): " << overlap << '\n';

    // Symmetric difference (exclusive regions)
    auto exclusive = segment1 ^ segment2;
    std::cout << "Exclusive regions (1 ^ 2): " << exclusive << '\n';

    // Measure calculations
    std::cout << "\nMeasures:\n";
    std::cout << "Total coverage: " << coverage.measure() << '\n';
    std::cout << "Overlap: " << overlap.measure() << '\n';
    std::cout << "Exclusive: " << exclusive.measure() << '\n';

    // Gap analysis
    auto gaps = coverage.gaps();
    std::cout << "\nGaps in coverage: " << gaps << '\n';
    std::cout << "Total gap measure: " << gaps.measure() << '\n';
}

int main() {
    demonstrate_geometric_operations();
    return 0;
}
```

---

## Numerical Analysis

### Example 4: Interval Arithmetic with Uncertainty

```cpp
#include <dis/dis.hpp>
#include <iostream>
#include <cmath>

using namespace dis;

class MeasurementWithUncertainty {
    double center_;
    double uncertainty_;

public:
    MeasurementWithUncertainty(double center, double uncertainty)
        : center_(center), uncertainty_(uncertainty) {}

    real_interval as_interval() const {
        return real_interval::closed(
            center_ - uncertainty_,
            center_ + uncertainty_
        );
    }

    static MeasurementWithUncertainty from_interval(real_interval interval) {
        double center = interval.midpoint();
        double uncertainty = interval.length() / 2.0;
        return MeasurementWithUncertainty(center, uncertainty);
    }

    MeasurementWithUncertainty operator+(const MeasurementWithUncertainty& other) const {
        auto i1 = as_interval();
        auto i2 = other.as_interval();

        double min = *i1.lower_bound() + *i2.lower_bound();
        double max = *i1.upper_bound() + *i2.upper_bound();

        return from_interval(real_interval::closed(min, max));
    }

    MeasurementWithUncertainty operator*(const MeasurementWithUncertainty& other) const {
        auto i1 = as_interval();
        auto i2 = other.as_interval();

        std::array products = {
            *i1.lower_bound() * *i2.lower_bound(),
            *i1.lower_bound() * *i2.upper_bound(),
            *i1.upper_bound() * *i2.lower_bound(),
            *i1.upper_bound() * *i2.upper_bound()
        };

        auto [min, max] = std::minmax_element(products.begin(), products.end());
        return from_interval(real_interval::closed(*min, *max));
    }

    friend std::ostream& operator<<(std::ostream& os, const MeasurementWithUncertainty& m) {
        os << m.center_ << " ± " << m.uncertainty_;
        return os;
    }
};

int main() {
    // Physical measurements with uncertainty
    MeasurementWithUncertainty voltage(5.0, 0.1);    // 5V ± 0.1V
    MeasurementWithUncertainty current(2.0, 0.05);   // 2A ± 0.05A

    std::cout << "Voltage: " << voltage << "V\n";
    std::cout << "Current: " << current << "A\n";

    // Calculate power with propagated uncertainty
    auto power = voltage * current;
    std::cout << "Power: " << power << "W\n";

    // Calculate total resistance in series
    MeasurementWithUncertainty r1(100, 5);  // 100Ω ± 5Ω
    MeasurementWithUncertainty r2(200, 10); // 200Ω ± 10Ω

    auto total_r = r1 + r2;
    std::cout << "\nR1: " << r1 << "Ω\n";
    std::cout << "R2: " << r2 << "Ω\n";
    std::cout << "Total: " << total_r << "Ω\n";

    return 0;
}
```

---

## Access Control

### Example 5: Time-Based Access Control System

```cpp
#include <dis/dis.hpp>
#include <map>
#include <string>
#include <iostream>

using namespace dis;

class AccessControlSystem {
    struct UserPolicy {
        real_set allowed_times;
        std::set<std::string> allowed_resources;
    };

    std::map<std::string, UserPolicy> policies_;

public:
    void add_user(const std::string& username) {
        policies_[username] = UserPolicy{};
    }

    void grant_time_access(const std::string& username,
                          double start, double end) {
        policies_[username].allowed_times.insert(
            real_interval::closed(start, end)
        );
    }

    void grant_resource_access(const std::string& username,
                              const std::string& resource) {
        policies_[username].allowed_resources.insert(resource);
    }

    void revoke_time_access(const std::string& username,
                           double start, double end) {
        policies_[username].allowed_times.erase(
            real_interval::closed(start, end)
        );
    }

    bool can_access(const std::string& username,
                   const std::string& resource,
                   double time) {
        auto it = policies_.find(username);
        if (it == policies_.end()) return false;

        const auto& policy = it->second;
        return policy.allowed_times.contains(time) &&
               policy.allowed_resources.count(resource) > 0;
    }

    void print_policy(const std::string& username) {
        auto it = policies_.find(username);
        if (it == policies_.end()) {
            std::cout << "User not found\n";
            return;
        }

        const auto& policy = it->second;
        std::cout << "Policy for " << username << ":\n";
        std::cout << "  Allowed times: " << policy.allowed_times << '\n';
        std::cout << "  Allowed resources: ";
        for (const auto& resource : policy.allowed_resources) {
            std::cout << resource << " ";
        }
        std::cout << '\n';
    }
};

int main() {
    AccessControlSystem acs;

    // Setup users
    acs.add_user("alice");
    acs.add_user("bob");

    // Alice: Full business hours access
    acs.grant_time_access("alice", 9, 17);
    acs.grant_resource_access("alice", "database");
    acs.grant_resource_access("alice", "api");

    // Bob: Limited hours, lunch break excluded
    acs.grant_time_access("bob", 9, 12);
    acs.grant_time_access("bob", 13, 17);
    acs.grant_resource_access("bob", "database");

    // Print policies
    acs.print_policy("alice");
    acs.print_policy("bob");

    // Test access
    std::cout << "\nAccess Tests:\n";
    std::cout << "Alice database @ 10:00: "
              << (acs.can_access("alice", "database", 10) ? "GRANTED" : "DENIED") << '\n';
    std::cout << "Bob database @ 12:30: "
              << (acs.can_access("bob", "database", 12.5) ? "GRANTED" : "DENIED") << '\n';
    std::cout << "Bob api @ 14:00: "
              << (acs.can_access("bob", "api", 14) ? "GRANTED" : "DENIED") << '\n';

    return 0;
}
```

---

## Complete Application

### Example 6: Task Scheduler with Dependencies

```cpp
#include <dis/dis.hpp>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

using namespace dis;

struct Task {
    std::string name;
    double duration;
    std::vector<std::string> dependencies;
    real_interval scheduled_time;
};

class TaskScheduler {
    std::vector<Task> tasks_;
    std::map<std::string, real_interval> completion_times_;
    real_set occupied_slots_;
    real_set available_slots_;

public:
    TaskScheduler(double start_time, double end_time)
        : available_slots_(real_set{real_interval::closed(start_time, end_time)})
    {}

    void add_task(const std::string& name,
                 double duration,
                 const std::vector<std::string>& dependencies = {}) {
        tasks_.push_back({name, duration, dependencies, real_interval::empty()});
    }

    bool schedule_all() {
        // Topological sort for dependencies
        std::vector<Task*> ready_tasks;

        for (auto& task : tasks_) {
            if (task.dependencies.empty()) {
                ready_tasks.push_back(&task);
            }
        }

        while (!ready_tasks.empty()) {
            Task* task = ready_tasks.back();
            ready_tasks.pop_back();

            // Find earliest available slot
            double earliest_start = 0.0;

            // Check dependencies
            for (const auto& dep : task->dependencies) {
                if (auto it = completion_times_.find(dep);
                    it != completion_times_.end()) {
                    earliest_start = std::max(earliest_start,
                                            *it->second.upper_bound());
                }
            }

            // Find slot in available times
            for (const auto& slot : available_slots_) {
                double slot_start = *slot.lower_bound();
                if (slot_start >= earliest_start &&
                    slot.length() >= task->duration) {

                    // Schedule task
                    task->scheduled_time = real_interval::closed(
                        slot_start,
                        slot_start + task->duration
                    );

                    // Update occupied and available slots
                    occupied_slots_.insert(task->scheduled_time);
                    available_slots_ = available_slots_ - real_set{task->scheduled_time};

                    // Mark completion time
                    completion_times_[task->name] = task->scheduled_time;

                    // Check if this completes dependencies for other tasks
                    for (auto& other : tasks_) {
                        if (other.scheduled_time.is_empty() &&
                            std::find(other.dependencies.begin(),
                                    other.dependencies.end(),
                                    task->name) != other.dependencies.end()) {

                            // Check if all dependencies are done
                            bool all_deps_done = true;
                            for (const auto& dep : other.dependencies) {
                                if (completion_times_.find(dep) ==
                                    completion_times_.end()) {
                                    all_deps_done = false;
                                    break;
                                }
                            }

                            if (all_deps_done) {
                                ready_tasks.push_back(&other);
                            }
                        }
                    }

                    break;
                }
            }
        }

        // Check if all tasks were scheduled
        return std::all_of(tasks_.begin(), tasks_.end(),
                         [](const Task& t) { return !t.scheduled_time.is_empty(); });
    }

    void print_schedule() {
        std::cout << "Schedule:\n";
        for (const auto& task : tasks_) {
            if (!task.scheduled_time.is_empty()) {
                std::cout << "  " << task.name << ": "
                         << task.scheduled_time << '\n';
            } else {
                std::cout << "  " << task.name << ": NOT SCHEDULED\n";
            }
        }

        std::cout << "\nUtilization: "
                 << (occupied_slots_.measure() /
                     (occupied_slots_ | available_slots_).span().length() * 100)
                 << "%\n";
    }
};

int main() {
    TaskScheduler scheduler(0, 10);  // 10-hour window

    // Add tasks with dependencies
    scheduler.add_task("setup", 1.0);
    scheduler.add_task("compile", 2.0, {"setup"});
    scheduler.add_task("test", 1.5, {"compile"});
    scheduler.add_task("deploy", 0.5, {"test"});
    scheduler.add_task("docs", 1.0, {"setup"});  // Parallel with compile

    if (scheduler.schedule_all()) {
        scheduler.print_schedule();
    } else {
        std::cout << "Failed to schedule all tasks\n";
    }

    return 0;
}
```

---

## Compile and Run

All examples can be compiled with:

```bash
g++ -std=c++17 -I./include example.cpp -o example
./example
```

For more examples, see the [examples/](https://github.com/yourusername/disjoint_interval_set/tree/master/examples) directory in the repository.
