#ifndef INTERVAL_H
#define INTERVAL_H

#include <cmath>
#include <limits>

static_assert(std::numeric_limits<double>::has_infinity, "`double` needs to have "
"positive infinity");
static_assert(std::numeric_limits<double>::is_iec559, "`double` needs to be IEEE754 "
"in order for -std::numeric_limits<double>::infinity() to equal negative infinity");

/* The `Interval` class represents an interval from `min` to `max` (both of type
`double`), and provides helper functions both open and closed intervals. */
struct Interval {
    constexpr static double DOUBLE_INF = std::numeric_limits<double>::infinity();

    /* Minimum and maximum values in the interval. */
    double min, max;

    /* Returns `true` if `d` is in the INCLUSIVE range [min, max]. */
    bool contains(double d) const {return min <= d && d <= max;}
    /* Returns `true` if `d` is in the EXCLUSIVE range (min, max). */
    bool surrounds(double d) const {return min < d && d < max;}
    /* Returns the value of `d` when it is clamped to the range [min, max]. */
    auto clamp(double d) const {return (d <= min ? min : (d >= max ? max : d));}

    /* Constructs an `Interval` from `min_` to `max_`. */
    Interval(double min_, double max_) : min{min_}, max{max_} {}

    /* Returns an empty interval. */
    static auto empty() {return Interval(DOUBLE_INF, -DOUBLE_INF);}

    /* Returns the interval of all non-negative integers: [0, DOUBLE_INF). */
    static auto nonnegative() {return Interval(0, DOUBLE_INF);}

    /* Returns the interval of all real numbers */
    static auto universe() {return Interval(-DOUBLE_INF, DOUBLE_INF);}
};

#endif