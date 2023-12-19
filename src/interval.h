#ifndef INTERVAL_H
#define INTERVAL_H

#include <iostream>
#include <cmath>
#include <limits>
#include <numeric>

static_assert(std::numeric_limits<double>::has_infinity, "`double` needs to have "
"positive infinity");
static_assert(std::numeric_limits<double>::is_iec559, "`double` needs to be IEEE754 "
"in order for -std::numeric_limits<double>::infinity() to equal negative infinity");

/* The `Interval` class represents an interval from `min` to `max` (both of type
`double`), and provides helper functions both open and closed intervals. */
struct Interval {
    constexpr static double DOUBLE_INF = std::numeric_limits<double>::infinity();

    /* `min`/`max` = Minimum/maximum values in the interval, respectively */
    double min, max;

    /* Returns the minimum bound of this `Interval` if `index` is 0, and otherwise returns the
    maximum bound of this `Interval`. */
    const auto& operator[] (size_t index) const {return (index ? max : min);}

    /* Returns the midpoint of this `Interval`; that is, `(min + max) / 2`. */
    auto midpoint() const {return std::midpoint(min, max);}
    /* Returns the size of this `Interval`; that is, `max - min`. */
    auto size() const {return max - min;}
    /* Returns `true` if this `Interval` is empty (that is, if `max - min <= 0`). */
    bool is_empty() const {return size() <= 0;}

    /* Returns `true` if `d` is in the INCLUSIVE range [min, max]. */
    bool contains_inclusive(double d) const {return min <= d && d <= max;}
    /* Returns `true` if `d` is in the EXCLUSIVE range (min, max). */
    bool contains_exclusive(double d) const {return min < d && d < max;}
    /* Returns the value of `d` when it is clamped to the range [min, max]. */
    auto clamp(double d) const {return (d <= min ? min : (d >= max ? max : d));}

    /* Updates (possibly expands) this `Interval` to also contain the `Interval` `other`. */
    void merge_with(const Interval &other) {
        min = std::fmin(min, other.min);
        max = std::fmax(max, other.max);
    }

    /* Updates (possibly expands) this `Interval` to contain `d`. */
    void merge_with(double d) {
        min = std::fmin(min, d);
        max = std::fmax(max, d);
    }

    /* Setters */

    /* Pads this interval with `padding`; that is, expands this `Interval` by `padding` on both
    ends. */
    auto& pad_with(double padding) {
        min -= padding;
        max += padding;
        return *this;
    }

    /* --- CONSTRUCTORS --- */

    /* Constructs an `Interval` from `min_` to `max_`. */
    Interval(double min_, double max_) : min{min_}, max{max_} {}

    /* --- NAMED CONSTRUCTORS --- */

    /* Returns an empty interval; specifically, the interval `(DOUBLE_INF, -DOUBLE_INF)`.
    The rationale behind using `(DOUBLE_INF, -DOUBLE_INF)` is that it allows for easier
    computation of intersections of intervals, which is needed in `AABB::is_hit_by()`.*/
    static auto empty() {return Interval(DOUBLE_INF, -DOUBLE_INF);}

    /* Returns the interval of all non-negative integers: `[0, DOUBLE_INF)`. */
    static auto nonnegative() {return Interval(0, DOUBLE_INF);}

    /* Returns the interval with minimum `min_` and maximum `DOUBLE_INF`. */
    static auto with_min(double min_) {return Interval(min_, DOUBLE_INF);}

    /* Returns the interval with maximum `max_` and minimum `-DOUBLE_INF`. */
    static auto with_max(double max_) {return Interval(-DOUBLE_INF, max_);}

    /* Returns the interval of all real numbers */
    static auto universe() {return Interval(-DOUBLE_INF, DOUBLE_INF);}

    /* Returns the minimum-size interval that fully contains both of the intervals `a` and `b`;
    that is, returns the interval that would result if `a` and `b` were to be merged into a single
    interval. Thus, this returns the interval from `min(a.min, b.min)` to `max(a.max, b.max)`. */
    static auto merge(const Interval &a, const Interval &b) {
        return Interval(std::fmin(a.min, b.min), std::fmax(a.max, b.max));
    }
};

/* Overload `operator<<` to allow printing `Interval`s to output streams */
std::ostream& operator<< (std::ostream &os, const Interval &i) {
    os << "Interval {min: " << i.min << ", max: " << i.max << "} ";
    return os;
}

#endif