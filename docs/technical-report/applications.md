# Applications

The DIS library excels in domains requiring interval manipulation and set-theoretic reasoning. This section presents real-world applications across multiple domains.

## Computational Geometry

### 1D Geometric Operations

Interval sets naturally represent 1D geometric objects:

```cpp
// Line segment operations
auto segment1 = real_set{real_interval::closed(0, 10)};
auto segment2 = real_set{real_interval::closed(5, 15)};

auto overlap = segment1 & segment2;        // [5, 10]
auto coverage = segment1 | segment2;       // [0, 15]
auto gap = segment1 ^ segment2;            // [0, 5) ∪ (10, 15]
```

### Polygon Clipping

Using scanline algorithm with interval sets:

```cpp
std::map<double, real_set> scanlines;  // y-coordinate → x-intervals

// For each polygon edge, update scanlines
for (const auto& edge : polygon.edges()) {
    double y_start = edge.min_y();
    double y_end = edge.max_y();

    // Add x-interval at this y-coordinate
    scanlines[y_start].insert(real_interval::closed(
        edge.x_at_y(y_start),
        edge.x_at_y(y_end)
    ));
}

// Boolean operations on polygons become set operations on scanlines
auto clip_polygon = [](const auto& p1, const auto& p2) {
    std::map<double, real_set> result;
    for (auto y : all_y_coordinates) {
        result[y] = p1.scanlines[y] & p2.scanlines[y];
    }
    return result;
};
```

### Constructive Solid Geometry (CSG)

Build complex shapes via set operations:

```cpp
// 2D example: Create a rounded rectangle
auto rectangle = real_set::from_string("[0,100]");
auto left_round = /* circular cutout */;
auto right_round = /* circular cutout */;

auto rounded_rect = (rectangle - left_round) - right_round;

// 3D example: Create a hollow sphere
auto outer_sphere = box<double>::sphere(origin, 10.0);
auto inner_sphere = box<double>::sphere(origin, 9.5);
auto shell = outer_sphere - inner_sphere;
```

---

## Scheduling and Resource Allocation

### Meeting Room Scheduling

```cpp
#include <dis/dis.hpp>
#include <map>
#include <string>

using namespace dis;

class RoomScheduler {
    std::map<std::string, real_set> room_schedules_;
    real_set business_hours_;

public:
    RoomScheduler()
        : business_hours_(real_set{}
            .add(9, 12)   // Morning
            .add(14, 17)) // Afternoon
    {}

    bool book_meeting(const std::string& room,
                     double start, double end) {
        auto meeting = real_interval::closed(start, end);

        // Check if room is available
        if (!room_schedules_[room].overlaps(real_set{meeting})) {
            room_schedules_[room].insert(meeting);
            return true;
        }
        return false;
    }

    real_set find_free_slots(const std::string& room,
                             double min_duration) const {
        auto available = business_hours_;

        if (auto it = room_schedules_.find(room);
            it != room_schedules_.end()) {
            available = available - it->second;
        }

        // Filter slots by minimum duration
        return available.filter([min_duration](const auto& slot) {
            return slot.length() >= min_duration;
        });
    }

    std::vector<std::string> find_rooms_available(double start, double end) const {
        auto required = real_interval::closed(start, end);
        std::vector<std::string> available_rooms;

        for (const auto& [room, schedule] : room_schedules_) {
            if (!schedule.overlaps(real_set{required})) {
                available_rooms.push_back(room);
            }
        }

        return available_rooms;
    }

    double utilization(const std::string& room) const {
        if (auto it = room_schedules_.find(room);
            it != room_schedules_.end()) {
            return it->second.measure() / business_hours_.measure();
        }
        return 0.0;
    }
};

// Usage
RoomScheduler scheduler;
scheduler.book_meeting("Room A", 10, 11);
scheduler.book_meeting("Room A", 15, 16);

auto free_slots = scheduler.find_free_slots("Room A", 1.0);
std::cout << "Free slots (≥1 hour): " << free_slots << '\n';
// Output: [9,10) ∪ (11,12] ∪ [14,15) ∪ (16,17]
```

### CPU/GPU Resource Allocation

```cpp
class ResourceAllocator {
    real_set allocated_time_;

public:
    bool can_allocate(double start, double duration) const {
        auto request = real_interval::closed(start, start + duration);
        return !allocated_time_.overlaps(real_set{request});
    }

    void allocate(double start, double duration) {
        allocated_time_.insert(
            real_interval::closed(start, start + duration)
        );
    }

    real_set find_next_available(double min_duration) const {
        auto free = ~allocated_time_;  // Complement
        return free.filter([min_duration](const auto& slot) {
            return slot.length() >= min_duration;
        });
    }
};
```

---

## Numerical Analysis

### Interval Arithmetic for Error Bounds

```cpp
class UncertainValue {
    real_set possible_values_;

public:
    UncertainValue(double center, double uncertainty)
        : possible_values_(real_set{
            real_interval::closed(center - uncertainty,
                                 center + uncertainty)
        }) {}

    UncertainValue(real_set values)
        : possible_values_(std::move(values)) {}

    UncertainValue operator+(const UncertainValue& other) const {
        real_set result;
        for (const auto& i1 : possible_values_) {
            for (const auto& i2 : other.possible_values_) {
                double min = *i1.lower_bound() + *i2.lower_bound();
                double max = *i1.upper_bound() + *i2.upper_bound();
                result.insert(real_interval::closed(min, max));
            }
        }
        return UncertainValue(std::move(result));
    }

    UncertainValue operator*(const UncertainValue& other) const {
        real_set result;
        for (const auto& i1 : possible_values_) {
            for (const auto& i2 : other.possible_values_) {
                auto products = std::array{
                    *i1.lower_bound() * *i2.lower_bound(),
                    *i1.lower_bound() * *i2.upper_bound(),
                    *i1.upper_bound() * *i2.lower_bound(),
                    *i1.upper_bound() * *i2.upper_bound()
                };
                auto [min, max] = std::minmax_element(
                    products.begin(), products.end()
                );
                result.insert(real_interval::closed(*min, *max));
            }
        }
        return UncertainValue(std::move(result));
    }

    real_interval bounds() const {
        return possible_values_.span();
    }

    double uncertainty() const {
        auto span = possible_values_.span();
        return span.length() / 2.0;
    }
};

// Usage: Propagate measurement uncertainty
UncertainValue voltage(5.0, 0.1);    // 5V ± 0.1V
UncertainValue current(2.0, 0.05);   // 2A ± 0.05A
auto power = voltage * current;      // ~10W with propagated uncertainty

std::cout << "Power: " << power.bounds() << '\n';
std::cout << "Uncertainty: ±" << power.uncertainty() << "W\n";
```

### Range Analysis in Program Verification

```cpp
// Track possible values of variables during symbolic execution
std::map<std::string, real_set> variable_ranges;

// After: x = [5, 10]; y = [0, 3];
variable_ranges["x"] = real_set::from_string("[5,10]");
variable_ranges["y"] = real_set::from_string("[0,3]");

// Analyze: z = x + y
auto z_range = /* compute from x and y ranges */;
variable_ranges["z"] = z_range;  // [5, 13]

// Check assertion: assert(z > 0)
bool assertion_holds = !z_range.overlaps(
    real_set{real_interval::at_most(0)}
);
```

---

## Access Control

### Time-Based Access Control

```cpp
class AccessPolicy {
    real_set allowed_hours_;
    std::set<std::string> allowed_users_;

public:
    AccessPolicy()
        : allowed_hours_(real_set{}
            .add(9, 17)  // Business hours
            .remove(real_interval::closed(12, 13))  // Lunch break
        ) {}

    bool can_access(const std::string& user, double time) const {
        return allowed_users_.contains(user) &&
               allowed_hours_.contains(time);
    }

    real_set get_access_windows(const std::string& user) const {
        if (allowed_users_.contains(user)) {
            return allowed_hours_;
        }
        return real_set::empty();
    }

    void add_exception(double start, double end) {
        // Add special access window (e.g., maintenance)
        allowed_hours_.insert(real_interval::closed(start, end));
    }
};
```

### IP Address Range Management

```cpp
// IPv4 addresses as 32-bit integers
using ip_set = disjoint_interval_set<interval<uint32_t>>;

class FirewallRules {
    ip_set allowed_ips_;
    ip_set blocked_ips_;

public:
    void allow_range(uint32_t start, uint32_t end) {
        allowed_ips_.insert(integer_interval::closed(start, end));
    }

    void block_range(uint32_t start, uint32_t end) {
        blocked_ips_.insert(integer_interval::closed(start, end));
    }

    bool is_allowed(uint32_t ip) const {
        return allowed_ips_.contains(ip) &&
               !blocked_ips_.contains(ip);
    }

    // Get effective allowed IPs (allowed - blocked)
    ip_set effective_allowed() const {
        return allowed_ips_ - blocked_ips_;
    }
};

// Usage
FirewallRules fw;
fw.allow_range(ip_to_uint("10.0.0.0"), ip_to_uint("10.255.255.255"));
fw.block_range(ip_to_uint("10.0.1.0"), ip_to_uint("10.0.1.255"));

if (fw.is_allowed(ip_to_uint("10.0.2.5"))) {
    // Allow connection
}
```

---

## Data Analysis and Visualization

### Histogram Binning

```cpp
class Histogram {
    std::map<real_interval, size_t> bins_;

public:
    void add_bin(double start, double end) {
        bins_[real_interval::closed(start, end)] = 0;
    }

    void add_value(double value) {
        for (auto& [interval, count] : bins_) {
            if (interval.contains(value)) {
                ++count;
                break;
            }
        }
    }

    void print() const {
        for (const auto& [interval, count] : bins_) {
            std::cout << interval << ": "
                     << std::string(count, '*') << '\n';
        }
    }
};
```

### Range Query Optimization

```cpp
class TimeSeriesIndex {
    std::map<real_interval, std::vector<DataPoint>> index_;

public:
    void index_data(const std::vector<DataPoint>& data) {
        // Partition data into intervals
        for (const auto& point : data) {
            auto interval = real_interval::closed(
                std::floor(point.timestamp),
                std::ceil(point.timestamp)
            );
            index_[interval].push_back(point);
        }
    }

    std::vector<DataPoint> query_range(double start, double end) {
        auto query = real_interval::closed(start, end);
        std::vector<DataPoint> results;

        for (const auto& [interval, points] : index_) {
            if (interval.overlaps(query)) {
                // Filter points within exact range
                for (const auto& point : points) {
                    if (query.contains(point.timestamp)) {
                        results.push_back(point);
                    }
                }
            }
        }

        return results;
    }
};
```

---

## Signal Processing

### Frequency Band Allocation

```cpp
class SpectrumManager {
    real_set allocated_bands_;
    const double min_freq = 0.0;
    const double max_freq = 6000.0;  // MHz

public:
    bool allocate_band(double start, double end) {
        auto band = real_interval::closed(start, end);

        if (!allocated_bands_.overlaps(real_set{band})) {
            allocated_bands_.insert(band);
            return true;
        }
        return false;
    }

    real_set find_available_bands(double bandwidth) const {
        auto available = real_set{real_interval::closed(min_freq, max_freq)}
                        - allocated_bands_;

        return available.filter([bandwidth](const auto& band) {
            return band.length() >= bandwidth;
        });
    }

    double spectrum_efficiency() const {
        return allocated_bands_.measure() / (max_freq - min_freq);
    }
};

// Usage: Allocate WiFi channels
SpectrumManager wifi;
wifi.allocate_band(2412, 2437);  // Channel 1-6
wifi.allocate_band(5180, 5320);  // 5GHz band

auto available = wifi.find_available_bands(20);  // Need 20MHz
```

---

## Game Development

### Collision Detection (1D)

```cpp
// Simple platformer collision detection
struct Entity {
    real_interval x_bounds;
    real_interval y_bounds;

    bool collides_with(const Entity& other) const {
        return x_bounds.overlaps(other.x_bounds) &&
               y_bounds.overlaps(other.y_bounds);
    }
};

// Level editor: Track occupied space
class LevelEditor {
    disjoint_interval_set<real_interval> occupied_x_;
    disjoint_interval_set<real_interval> occupied_y_;

public:
    bool can_place(double x, double y, double width, double height) {
        auto x_range = real_interval::closed(x, x + width);
        auto y_range = real_interval::closed(y, y + height);

        return !occupied_x_.overlaps(real_set{x_range}) &&
               !occupied_y_.overlaps(real_set{y_range});
    }

    void place(double x, double y, double width, double height) {
        occupied_x_.insert(real_interval::closed(x, x + width));
        occupied_y_.insert(real_interval::closed(y, y + height));
    }
};
```

---

## Summary

The DIS library's mathematical foundation and elegant API make it suitable for:

- **Geometric algorithms**: CSG, clipping, intersection testing
- **Resource management**: Scheduling, allocation, conflict detection
- **Numerical methods**: Error bounds, range analysis, uncertainty propagation
- **Security systems**: Access control, IP filtering, time-based permissions
- **Data analysis**: Binning, range queries, aggregation
- **Signal processing**: Spectrum management, band allocation
- **Game development**: Collision detection, spatial indexing

The common thread: **problems naturally expressible as set operations on intervals benefit from DIS's mathematical rigor and ergonomic API**.
